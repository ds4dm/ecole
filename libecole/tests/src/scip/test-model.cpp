#include <future>
#include <limits>
#include <string>

#include <catch2/catch.hpp>
#include <scip/scip.h>

#include "ecole/scip/exception.hpp"
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
	BMScheckEmptyMemory();
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

TEST_CASE("Equality comparison") {
	auto model = scip::Model{};
	REQUIRE(model == model);
	REQUIRE(model != scip::Model{model});
}

TEST_CASE("Create model from file") {
	auto model = scip::Model::from_file(problem_file);
}

TEST_CASE("Raise if file does not exist") {
	auto guard = ScipNoErrorGuard{};
	REQUIRE_THROWS_AS(scip::Model::from_file("/does_not_exist.mps"), scip::Exception);
}

TEST_CASE("Model solving") {
	SECTION("Synchronously") {
		auto model = get_model();
		model.solve();
	}

	SECTION("Asynchronously") {
		auto load_solve = [] {
			get_model().solve();
			return true;
		};
		auto fut1 = std::async(std::launch::async, load_solve);
		auto fut2 = std::async(std::launch::async, load_solve);
		REQUIRE((fut1.get() && fut2.get()));
	}
}

TEST_CASE("Copy preserve the model internals") {
	auto model = get_model();
	auto model_copy = model;
	model.solve();
	model_copy.solve();
}

TEST_CASE("Get and set parameters") {
	using scip::ParamType;

	auto model = scip::Model{};
	auto constexpr param = "conflict/conflictgraphweight";

	SECTION("Get parameters explicitly") {
		model.get_param_explicit<ParamType::Real>(param);
	}

	SECTION("Set parameters explicitly") {
		model.set_param_explicit<ParamType::Real>(param, false);
	}

	SECTION("Throw on wrong parameters type") {
		auto guard = ScipNoErrorGuard{};
		REQUIRE_THROWS_AS(model.get_param_explicit<ParamType::Int>(param), scip::Exception);
		REQUIRE_THROWS_AS(
			model.set_param_explicit<ParamType::Int>(param, 3), scip::Exception);
	}

	SECTION("Get parameters with automatic casting") { model.get_param<int>(param); }

	SECTION("Set parameters with automatic casting") { model.set_param(param, 1); }

	SECTION("String parameters can be converted to chars") {
		model.set_param("branching/scorefunc", "s");
		REQUIRE(model.get_param<char>("branching/scorefunc") == 's');
	}
}
