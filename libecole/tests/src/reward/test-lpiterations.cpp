#include <memory>
#include <string>

#include <catch2/catch.hpp>

#include "ecole/reward/lpiterations.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("LpIterations unit tests", "[unit][reward]") {
	reward::unit_tests(reward::LpIterations{});
}

TEST_CASE("LpIterations return the difference in LP iterations between two states", "[reward]") {
	auto reward_func = reward::LpIterations{};
	auto model = get_solving_model();
	reward_func.reset(model);

	SECTION("LP iterations are positive") { REQUIRE(reward_func.obtain_reward(model) > 0); }

	SECTION("LP iterations is zero if no solving happended between two states") {
		reward_func.obtain_reward(model);
		REQUIRE(reward_func.obtain_reward(model) == 0);
	}

	SECTION("Reset LP iterations coutner") {
		reward_func.obtain_reward(model);
		REQUIRE(reward_func.obtain_reward(model) == 0);
		reward_func.reset(model);
		REQUIRE(reward_func.obtain_reward(model) > 0);
	}

	SECTION("No iterations if SCIP is not solving LPs") {
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
