#include <stdexcept>
#include <tuple>

#include <catch2/catch.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xsort.hpp>

#include "ecole/environment/branching-dynamics.hpp"
#include "ecole/environment/exception.hpp"

#include "conftest.hpp"
#include "environment/unit-tests.hpp"

using namespace ecole;

TEST_CASE("BranchingDynamics unit tests", "[unit][dyn]") {
	bool pseudo_candidates = GENERATE(true, false);
	auto const policy = [](auto const& action_set) { return action_set.value()[0]; };
	environment::unit_tests(environment::BranchingDynamics{pseudo_candidates}, policy);
}

TEST_CASE("BranchingDynamics functional tests", "[dyn]") {
	bool pseudo_candidates = GENERATE(true, false);
	environment::BranchingDynamics dyn{pseudo_candidates};
	auto model = get_model();
	bool done = false;
	trait::action_set_of_t<environment::BranchingDynamics> action_set;

	SECTION("Return valid action set") {
		std::tie(std::ignore, action_set) = dyn.reset_dynamics(model);
		REQUIRE(action_set.has_value());
		auto const& branch_cands = action_set.value();
		REQUIRE(branch_cands.size() > 0);
		REQUIRE(branch_cands.size() < model.lp_columns().size);
		REQUIRE(xt::all(branch_cands >= 0));
		REQUIRE(xt::all(branch_cands < model.lp_columns().size));
		REQUIRE(xt::unique(branch_cands).size() == branch_cands.size());
	}

	SECTION("Solve instance") {
		std::tie(done, action_set) = dyn.reset_dynamics(model);
		while (!done) {
			REQUIRE(action_set.has_value());
			std::tie(done, action_set) = dyn.step_dynamics(model, action_set.value()[0]);
		}
		REQUIRE(model.is_solved());
	}

	SECTION("Throw on invalid branching variable") {
		std::tie(done, action_set) = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
		REQUIRE(action_set.has_value());
		auto const action = model.lp_columns().size + 1;
		REQUIRE_THROWS_AS(dyn.step_dynamics(model, action), environment::Exception);
	}
}
