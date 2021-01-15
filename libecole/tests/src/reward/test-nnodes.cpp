#include <memory>
#include <string>
#include <tuple>

#include <catch2/catch.hpp>

#include "ecole/reward/nnodes.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("NNodes unit tests", "[unit][reward]") {
	reward::unit_tests(reward::NNodes{});
}

TEST_CASE("NNodes returns the difference in the total number of processed nodes between two states", "[reward]") {

	auto reward_func = reward::NNodes{};
	auto model = get_model();  // a non-trivial instance is loaded

	SECTION("NNodes is zero before presolving") {
		reward_func.before_reset(model);
		REQUIRE(reward_func.extract(model) == 0);
	}

	SECTION("NNodes is one after root node processing") {
		reward_func.before_reset(model);
		advance_to_root_node(model);
		REQUIRE(reward_func.extract(model) == 1);
	}

	SECTION("NNodes is zero if the model state has not changed") {
		reward_func.before_reset(model);
		advance_to_root_node(model);
		REQUIRE(reward_func.extract(model) >= 0);
		REQUIRE(reward_func.extract(model) == 0);
	}

	SECTION("Reset NNodes counter") {
		reward_func.before_reset(model);
		advance_to_root_node(model);
		auto reward = reward_func.extract(model);
		model = get_model();
		reward_func.before_reset(model);
		advance_to_root_node(model);
		REQUIRE(reward_func.extract(model) == reward);
	}
}
