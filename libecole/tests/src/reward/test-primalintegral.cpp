#include <catch2/catch.hpp>

#include "ecole/reward/primalintegral.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("PrimalIntegral unit tests", "[unit][reward]") {
	reward::unit_tests(reward::PrimalIntegral{});
}

TEST_CASE("PrimalIntegral returns the difference in dual integral between two states", "[reward]") {
	auto reward_func = reward::PrimalIntegral{};
	auto model = get_model();  // a non-trivial instance is loaded

	SECTION("PrimalIntegral is non-negative before presolving") {
		reward_func.before_reset(model);
		REQUIRE(reward_func.extract(model) >= 0);
	}
}
