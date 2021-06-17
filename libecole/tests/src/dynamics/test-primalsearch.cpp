#include <stdexcept>
#include <tuple>

#include <catch2/catch.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xsort.hpp>

#include <scip/scip.h>

#include "ecole/dynamics/primalsearch.hpp"
#include "ecole/exception.hpp"

#include "conftest.hpp"
#include "dynamics/unit-tests.hpp"

using namespace ecole;

TEST_CASE("PrimalSearchDynamics unit tests", "[unit][dynamics]") {
	dynamics::unit_tests(
		dynamics::PrimalSearchDynamics{},
		[](auto const& /*action_set*/, auto const& /*model*/) -> dynamics::PrimalSearchDynamics::Action {
			return {{}, {}};
		});
}

TEST_CASE("PrimalSearchDynamics functional tests", "[dynamics]") {
	const auto trials_per_node = 5;
	auto dyn = dynamics::PrimalSearchDynamics{trials_per_node};
	auto model = get_model();

	SECTION("Return valid action set") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE(action_set.has_value());
		auto var_ids = action_set.value();
		REQUIRE(var_ids.size() > 0);
		REQUIRE(var_ids.size() <= model.variables().size());
		REQUIRE(xt::all(var_ids >= 0));
		REQUIRE(xt::all(var_ids < model.variables().size()));
		REQUIRE(xt::unique(var_ids).size() == var_ids.size());
	}

	SECTION("Handle extreme action - empty") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE(action_set.has_value());
		auto var_ids = action_set.value();
		REQUIRE(var_ids.size() > 0);
		std::tie(done, action_set) = dyn.step_dynamics(model, {{}, {}});
		REQUIRE_FALSE(done);
	}

	SECTION("Handle extreme values") {
		auto const scip_inf = SCIPinfinity(model.get_scip_ptr());
		auto const scip_unknown = SCIP_UNKNOWN;
		auto const scip_invalid = SCIP_INVALID;
		auto const scip_min = std::numeric_limits<SCIP_Real>::min();
		auto const scip_max = std::numeric_limits<SCIP_Real>::max();

		auto const val = GENERATE_COPY(scip_min, scip_max, scip_unknown, scip_invalid, -scip_inf, scip_inf);

		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE(action_set.has_value());
		auto var_ids = action_set.value();
		REQUIRE(var_ids.size() > 0);
		auto var_vals = std::vector<SCIP_Real>(var_ids.size(), val);
		auto action = dynamics::PrimalSearchDynamics::Action{
			nonstd::span<std::size_t>{var_ids.data(), var_ids.size()},
			nonstd::span<SCIP_Real>{var_vals.data(), var_vals.size()}};
		std::tie(done, action_set) = dyn.step_dynamics(model, action);

		REQUIRE_FALSE(done);
	}

	SECTION("Solve instance") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		while (!done) {
			REQUIRE(action_set.has_value());
			std::tie(done, action_set) = dyn.step_dynamics(model, {{}, {}});
		}
		REQUIRE(model.is_solved());
	}

	SECTION("Throw on invalid variable id") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
		REQUIRE(action_set.has_value());
		auto var_ids = std::vector<std::size_t>{model.variables().size()};
		auto var_vals = std::vector<SCIP_Real>{0.0};
		auto action = dynamics::PrimalSearchDynamics::Action{
			nonstd::span<std::size_t>{var_ids.data(), var_ids.size()},
			nonstd::span<SCIP_Real>{var_vals.data(), var_vals.size()}};
		REQUIRE_THROWS_AS(dyn.step_dynamics(model, action), std::exception);
	}
}

TEST_CASE("PrimalSearchDynamics handles limits", "[dynamics]") {
	auto dyn = dynamics::PrimalSearchDynamics{1};
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
		std::tie(done, action_set) = dyn.step_dynamics(model, {{}, {}});
	}
}

TEST_CASE("PrimalSearchDynamics extreme parameterizations", "[dynamics]") {

	SECTION("Infinite trial loop") {
		auto dyn = dynamics::PrimalSearchDynamics{-1};
		auto model = get_model();

		auto const time_limit = GENERATE(0, 1, 2);
		model.set_param("limits/time", time_limit);  // we must set a time limit
		auto [done, action_set] = dyn.reset_dynamics(model);
		while (!done) {
			REQUIRE(action_set.has_value());
			std::tie(done, action_set) = dyn.step_dynamics(model, {{}, {}});
		}
	}

	SECTION("Zero trial") {
		auto dyn = dynamics::PrimalSearchDynamics{0};
		auto model = get_model();

		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE(done);
	}

	SECTION("Single trial at root node") {
		auto dyn = dynamics::PrimalSearchDynamics{1, 0};
		auto model = get_model();

		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
		std::tie(done, action_set) = dyn.step_dynamics(model, {{}, {}});
		REQUIRE(done);
	}

	SECTION("Single trial at root node (alternative)") {
		auto dyn = dynamics::PrimalSearchDynamics{1, 1, 0, 0};  // trials, freq, start, stop
		auto model = get_model();

		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
		std::tie(done, action_set) = dyn.step_dynamics(model, {{}, {}});
		REQUIRE(done);
	}

	SECTION("X trial per node") {
		auto const trials_per_node = GENERATE(1, 2, 3);
		auto dyn = dynamics::PrimalSearchDynamics{trials_per_node};
		auto model = get_model();
		model.set_param("limits/totalnodes", 3);
		int nsteps = 0;

		auto [done, action_set] = dyn.reset_dynamics(model);
		while (!done) {
			std::tie(done, action_set) = dyn.step_dynamics(model, {{}, {}});
			nsteps++;
		}
		REQUIRE(nsteps % trials_per_node == 0);
	}
}
