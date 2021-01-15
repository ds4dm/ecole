#include <catch2/catch.hpp>

#include "ecole/reward/isdone.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("IsDone unit tests", "[unit][reward]") {
	reward::unit_tests(reward::IsDone{});
}

TEST_CASE("IsDone always return one when done", "[reward]") {
	auto done = GENERATE(true, false);
	auto reward_func = reward::IsDone{};
	auto model = get_model();

	reward_func.before_reset(model);

	advance_to_root_node(model);

	REQUIRE(reward_func.extract(model, done) == (done ? 1. : 0.));

	SECTION("On successive calls") { REQUIRE(reward_func.extract(model, done) == (done ? 1. : 0.)); }
}
