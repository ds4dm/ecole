#include <catch2/catch.hpp>

#include "ecole/reward/solvingtime.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("SolvingTime unit tests", "[unit][reward]") {
	bool const wall = GENERATE(true, false);
	reward::unit_tests(reward::SolvingTime{wall});
}

TEST_CASE("Solving time rewards are positive initially", "[reward]") {
	bool const wall = GENERATE(true, false);
	auto reward_func = reward::SolvingTime{wall};
	auto model = get_model();  // a non-trivial instance is loaded

	SECTION("Solving time is nonnegative before presolving") {
		reward_func.reset(model);
		REQUIRE(reward_func.obtain_reward(model) >= 0);
	}

	SECTION("Solving time is stricly positive after root node processing") {
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		REQUIRE(reward_func.obtain_reward(model) > 0);
	}
}
