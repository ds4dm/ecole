#include <future>
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

TEST_CASE("Create model from file") { auto model = scip::Model::from_file(problem_file); }

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
		model.set_branch_rule([](auto const& model) { return scip::VarProxy::None; });
		model.solve();
		REQUIRE(model.is_solved());
	}

	SECTION("Exception in branching rule") {
		auto guard = ScipNoErrorGuard{};
		model.set_branch_rule([](auto const& model) -> scip::VarProxy { throw ":("; });
		REQUIRE_THROWS_AS(model.solve(), scip::Exception);
		REQUIRE(!model.is_solved());
	}
}
