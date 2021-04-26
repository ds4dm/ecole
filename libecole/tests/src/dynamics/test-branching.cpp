#include <stdexcept>
#include <tuple>

#include <catch2/catch.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xsort.hpp>

#include "ecole/dynamics/branching.hpp"
#include "ecole/exception.hpp"

#include "conftest.hpp"
#include "dynamics/unit-tests.hpp"

using namespace ecole;

TEST_CASE("BranchingDynamics unit tests", "[unit][dynamics]") {
	bool const pseudo_candidates = GENERATE(true, false);
	bool const branch_first = GENERATE(true, false);
	auto const policy = [branch_first](auto const& action_set) {
		auto const branch_idx = branch_first ? 0 : action_set.value().size() - 1;
		return action_set.value()[branch_idx];
	};
	dynamics::unit_tests(dynamics::BranchingDynamics{pseudo_candidates}, policy);
}

TEST_CASE("BranchingDynamics functional tests", "[dynamics]") {
	bool const pseudo_candidates = GENERATE(true, false);
	auto dyn = dynamics::BranchingDynamics{pseudo_candidates};
	auto model = get_model();

	SECTION("Return valid action set") {
		auto const [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE(action_set.has_value());
		auto const& branch_cands = action_set.value();
		REQUIRE(branch_cands.size() > 0);
		REQUIRE(branch_cands.size() <= model.lp_columns().size());
		REQUIRE(xt::all(branch_cands >= 0));
		REQUIRE(xt::all(branch_cands < model.lp_columns().size()));
		REQUIRE(xt::unique(branch_cands).size() == branch_cands.size());
	}

	SECTION("Solve instance") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		while (!done) {
			REQUIRE(action_set.has_value());
			std::tie(done, action_set) = dyn.step_dynamics(model, action_set.value()[0]);
		}
		REQUIRE(model.is_solved());
	}

	SECTION("Throw on invalid branching variable") {
		auto const [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
		REQUIRE(action_set.has_value());
		auto const action = model.lp_columns().size() + 1;
		REQUIRE_THROWS_AS(dyn.step_dynamics(model, action), std::exception);
	}
}

TEST_CASE("BranchingDynamics handles limits", "[dynamics]") {
	bool const pseudo_candidates = GENERATE(true, false);
	auto dyn = dynamics::BranchingDynamics{pseudo_candidates};
	auto model = get_model();

	SECTION("Node limit") {
		auto const node_limit = GENERATE(0, 1, 2);
		model.set_param("limits/totalnodes", node_limit);
	}

	SECTION("Time limit") {
		auto const time_limit = GENERATE(0, 1, 2);
		model.set_param("limits/time", time_limit);
	}

	auto [done, action_set] = dyn.reset_dynamics(model);
	while (!done) {
		REQUIRE(action_set.has_value());
		std::tie(done, action_set) = dyn.step_dynamics(model, action_set.value()[0]);
	}
}
