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
		reward_func.before_reset(model);
		REQUIRE(reward_func.extract(model) >= 0);
	}

	SECTION("Solving time is stricly positive after root node processing") {
		reward_func.before_reset(model);
		advance_to_root_node(model);
		REQUIRE(reward_func.extract(model) > 0);
	}
}
