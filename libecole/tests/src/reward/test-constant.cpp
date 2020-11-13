#include <catch2/catch.hpp>

#include "ecole/reward/constant.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("Constant reward unit tests", "[unit][reward]") {
	constexpr auto some_constant = 3.0;
	reward::unit_tests(reward::Constant{some_constant});
}

TEST_CASE("Constant reward always return the same value", "[reward]") {
	auto const done = GENERATE(true, false);
	auto const constant = GENERATE(-1., 0., 55);
	auto reward_func = reward::Constant{constant};
	auto model = get_solving_model();

	reward_func.reset(model);

	REQUIRE(reward_func.extract(model, done) == constant);

	SECTION("On successive calls") { REQUIRE(reward_func.extract(model, done) == constant); }
}
