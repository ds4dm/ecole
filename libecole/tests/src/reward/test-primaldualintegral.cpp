#include <catch2/catch.hpp>

#include "ecole/reward/primaldualintegral.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("PrimalDualIntegral unit tests", "[unit][reward]") {
	reward::unit_tests(reward::PrimalDualIntegral{});
}

TEST_CASE("DualInPrimalDualIntegraltegral returns the difference in dual integral between two states", "[reward]") {
	auto reward_func = reward::PrimalDualIntegral{};
	auto model = get_model();  // a non-trivial instance is loaded

	SECTION("PrimalDualIntegral is non-negative before presolving") {
		reward_func.before_reset(model);
		REQUIRE(reward_func.extract(model) >= 0);
	}
}
