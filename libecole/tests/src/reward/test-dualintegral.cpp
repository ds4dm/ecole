#include <catch2/catch.hpp>

#include "ecole/reward/dualintegral.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("DualIntegral unit tests", "[unit][reward]") {
	reward::unit_tests(reward::DualIntegral{});
}

TEST_CASE("DualIntegral returns the difference in dual integral between two states", "[reward]") {
	auto reward_func = reward::DualIntegral{};
	auto model = get_model();  // a non-trivial instance is loaded

	SECTION("DualIntegral iterations is 0 before presolving") {
		reward_func.before_reset(model);
    REQUIRE(reward_func.extract(model) == 0);
	}
	
}