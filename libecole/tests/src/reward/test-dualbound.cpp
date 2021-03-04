#include <catch2/catch.hpp>

#include "ecole/reward/dualbound.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("DualBound unit tests", "[unit][reward]") {
	reward::unit_tests(reward::DualBound{});
}

TEST_CASE("DualBound returns the dyal bound value", "[reward]") {
	auto reward_func = reward::DualBound{};
	auto model = get_model();  // a non-trivial instance is loaded

	SECTION("DualBound iterations is infinity before presolving") {
		reward_func.before_reset(model);
//    REQUIRE(reward_func.extract(model) == Infinity);
	}
	
	
	SECTION("No dual bound value if SCIP is not solving LPs") {
		model = get_model();
		model.set_params({
			{"presolving/maxrounds", 0},
			{"lp/iterlim", 0},
			{"lp/rootiterlim", 0},
			{"limits/totalnodes", 1},
		});
		advance_to_root_node(model);
		reward_func.before_reset(model);
//		REQUIRE(reward_func.extract(model) == Infinity);
	}
}