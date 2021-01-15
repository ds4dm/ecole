#include <catch2/catch.hpp>

#include "ecole/reward/lpiterations.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("LpIterations unit tests", "[unit][reward]") {
	reward::unit_tests(reward::LpIterations{});
}

TEST_CASE("LpIterations returns the difference in LP iterations between two states", "[reward]") {
	auto reward_func = reward::LpIterations{};
	auto model = get_model();  // a non-trivial instance is loaded

	SECTION("LP iterations is zero before presolving") {
		reward_func.before_reset(model);
		REQUIRE(reward_func.extract(model) == 0);
	}

	SECTION("LP iterations is stricly positive after root node processing") {
		reward_func.before_reset(model);
		advance_to_root_node(model);
		REQUIRE(reward_func.extract(model) > 0);
	}

	SECTION("LP iterations is zero if the model state has not changed") {
		reward_func.before_reset(model);
		advance_to_root_node(model);
		reward_func.extract(model);
		REQUIRE(reward_func.extract(model) == 0);
	}

	SECTION("Reset LP iteration counter") {
		reward_func.before_reset(model);
		advance_to_root_node(model);
		auto reward = reward_func.extract(model);
		model = get_model();
		reward_func.before_reset(model);
		advance_to_root_node(model);
		REQUIRE(reward_func.extract(model) == reward);
	}

	SECTION("No LP iterations if SCIP is not solving LPs") {
		model = get_model();
		model.set_params({
			{"presolving/maxrounds", 0},
			{"lp/iterlim", 0},
			{"lp/rootiterlim", 0},
			{"limits/totalnodes", 1},
		});
		advance_to_root_node(model);
		reward_func.before_reset(model);
		REQUIRE(reward_func.extract(model) == 0);
	}
}
