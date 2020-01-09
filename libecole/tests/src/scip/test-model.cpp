#include <future>
#include <limits>
#include <string>

#include <catch2/catch.hpp>
#include <scip/scip.h>

#include "ecole/exception.hpp"
#include "ecole/scip/model.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Allocation of ressources") {
	auto&& scip = scip::create();
	REQUIRE(SCIPgetStage(scip.get()) == SCIP_STAGE_INIT);
}

TEST_CASE("Dealocation of ressources") {
	auto scip = scip::create();
	scip.reset();
	REQUIRE(scip == nullptr);
	REQUIRE(BMSgetMemoryUsed() == 0);
}

TEST_CASE("Creation of model") {
	auto model = scip::Model{};
	SECTION("Copy construct") { auto model_copy = model; }
	SECTION("Move construct") { auto model_moved = std::move(model); }
}

TEST_CASE("Creation of model from scip pointer") {
	REQUIRE_THROWS_AS(scip::Model(nullptr), scip::Exception);
	scip::Model{scip::create()};
}

TEST_CASE("Create model from file") {
	auto model = scip::Model::from_file(problem_file);
}

TEST_CASE("Raise if file does not exist") {
	auto guard = ScipNoErrorGuard{};
	REQUIRE_THROWS_AS(scip::Model::from_file("/does_not_exist.mps"), scip::Exception);
}

TEST_CASE("Model modifiers") {
	auto model = scip::Model::from_file(problem_file);

	SECTION("Synchronously") { model.solve(); }

	SECTION("Asynchronously") {
		auto load_solve = [] {
			scip::Model::from_file(problem_file).solve();
			return true;
		};
		auto fut1 = std::async(std::launch::async, load_solve);
		auto fut2 = std::async(std::launch::async, load_solve);
		REQUIRE((fut1.get() && fut2.get()));
	}
}

TEST_CASE("Copy preserve the model internals") {
	auto model = scip::Model::from_file(problem_file);
	auto model_copy = model;
	model.solve();
	model_copy.solve();
}

TEST_CASE("Add a branching rule") {
	auto model = scip::Model::from_file(problem_file);
	model.disable_presolve();
	model.disable_cuts();
	auto count = 0;
	model.set_branch_rule([&count](scip::Model const& m) mutable {
		count++;
		return *m.lp_branch_vars().cbegin();
	});
	model.solve();
	REQUIRE(count > 0);
}

TEST_CASE("Run a branching rule") {
	auto model = scip::Model::from_file(problem_file);
	model.disable_cuts();
	model.disable_presolve();

	SECTION("Arbitrary branching rule") {
		model.set_branch_rule([](auto const& model) { return model.lp_branch_vars()[0]; });
		model.solve();
		REQUIRE(model.is_solved());
	}

	SECTION("Void branching rule") {
		model.set_branch_rule([](auto const&) { return scip::VarProxy::None; });
		model.solve();
		REQUIRE(model.is_solved());
	}

	SECTION("Exception in branching rule") {
		auto guard = ScipNoErrorGuard{};
		model.set_branch_rule([](auto const&) -> scip::VarProxy { throw ":("; });
		REQUIRE_THROWS_AS(model.solve(), scip::Exception);
		REQUIRE(!model.is_solved());
	}
}

TEST_CASE("Get and set parameters") {
	auto model = scip::Model{};
	auto constexpr param = "conflict/conflictgraphweight";

	SECTION("Get parameters explicitly") { model.get_param_explicit<double>(param); }

	SECTION("Set parameters explicitly") { model.set_param_explicit<double>(param, false); }

	SECTION("Throw on wrong parameters type") {
		auto guard = ScipNoErrorGuard{};
		REQUIRE_THROWS_AS(model.get_param_explicit<int>(param), scip::Exception);
		REQUIRE_THROWS_AS(model.set_param_explicit<int>(param, 3), scip::Exception);
	}

	SECTION("Get parameters with automatic casting") { model.get_param<int>(param); }

	SECTION("Set parameters with automatic casting") { model.set_param(param, 1); }

	SECTION("String parameters can be convert to chars") {
		model.set_param("branching/scorefunc", "s");
		REQUIRE(model.get_param<char>("branching/scorefunc") == 's');
	}
}

TEST_CASE("Seed the model") {
	auto model = scip::Model{};

	SECTION("Seed the model") {
		model.seed(42);
		REQUIRE(model.seed() == 42);
	}

	SECTION("Handle Scip limits") {
		model.seed(-1);
		REQUIRE(model.seed() > 0);
		model.seed(std::numeric_limits<scip::param_t<scip::ParamType::Int>>::max());
	}
}
