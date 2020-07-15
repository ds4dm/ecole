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
		reward_func.reset(model);
		REQUIRE(reward_func.obtain_reward(model) == 0);
	}

	SECTION("LP iterations is stricly positive after root node processing") {
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		REQUIRE(reward_func.obtain_reward(model) > 0);
	}

	SECTION("LP iterations is zero if the model state has not changed") {
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		reward_func.obtain_reward(model);
		REQUIRE(reward_func.obtain_reward(model) == 0);
	}

	SECTION("Reset LP iteration counter") {
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		auto reward = reward_func.obtain_reward(model);
		model = get_model();
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		REQUIRE(reward_func.obtain_reward(model) == reward);
	}

	SECTION("No LP iterations if SCIP is not solving LPs") {
		model = get_model();
		model.set_params({
			{"presolving/maxrounds", 0},
			{"lp/iterlim", 0},
			{"lp/rootiterlim", 0},
			{"limits/totalnodes", 1},
		});
		model.solve_iter();
		reward_func.reset(model);
		REQUIRE(reward_func.obtain_reward(model) == 0);
	}
}
