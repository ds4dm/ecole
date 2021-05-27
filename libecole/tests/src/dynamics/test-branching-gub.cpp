#include <array>
#include <random>
#include <stdexcept>
#include <tuple>
#include <vector>

#include <catch2/catch.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xsort.hpp>

#include "ecole/dynamics/branching-gub.hpp"
#include "ecole/exception.hpp"
#include "ecole/random.hpp"
#include "ecole/traits.hpp"

#include "conftest.hpp"
#include "dynamics/unit-tests.hpp"

using namespace ecole;

TEST_CASE("BranchingGUBDynamics unit tests with single branching", "[unit][dynamics]") {
	auto const policy = [](auto const& action_set, auto const& /*model*/) { return std::array{action_set.value()[0]}; };
	dynamics::unit_tests(dynamics::BranchingGUBDynamics{}, policy);
}

/**
 * A policy for multivariable branching.
 *
 * Try randomly to find two variables whose LP solution value sum is not integer.
 * As it may not always exists, stop otherwise and branch on a single variable.
 */
struct MultiBranchingPolicy {
	std::size_t n_multi = 0;
	RandomEngine rng{0};  // NOLINT(cert-msc32-c, cert-msc51-cpp) We want reproducibility in tests

	auto operator()(trait::action_set_of_t<dynamics::BranchingGUBDynamics> const& action_set, scip::Model& model)
		-> std::vector<std::size_t> {
		auto const& as = action_set.value();
		auto choice = std::uniform_int_distribution<std::size_t>{0, as.size() - 1};
		auto indices = std::array{as[choice(rng)], as[choice(rng)]};
		auto const is_lp_sum_integral = [&model](auto idx1, auto idx2) {
			auto const idx_to_var = [&](auto idx) { return SCIPcolGetVar(model.lp_columns()[idx]); };
			auto const is_lp = SCIPhasCurrentNodeLP(model.get_scip_ptr());
			auto const sum = SCIPvarGetSol(idx_to_var(idx1), is_lp) + SCIPvarGetSol(idx_to_var(idx2), is_lp);
			return SCIPfeasFloor(model.get_scip_ptr(), sum) == SCIPfeasCeil(model.get_scip_ptr(), sum);
		};
		auto constexpr n_trials = 10;
		for (std::size_t n = 0; n < n_trials; ++n) {
			if ((indices[0] != indices[1]) && (!is_lp_sum_integral(indices[0], indices[1]))) {
				n_multi++;
				return {indices[0], indices[1]};
			}
			indices = {as[choice(rng)], as[choice(rng)]};
		}
		return {indices[0]};
	}
};

TEST_CASE("BranchingGUBDynamics unit tests with multi branching", "[unit][dynamics]") {
	auto policy = MultiBranchingPolicy{};
	dynamics::unit_tests(dynamics::BranchingGUBDynamics{}, policy);
}

TEST_CASE("BranchingGUBDynamics can solve instance", "[dynamics][slow]") {
	auto dyn = dynamics::BranchingGUBDynamics{};
	auto model = get_model();
	auto policy = MultiBranchingPolicy{};

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

	SECTION("Solve instance with multiple variables") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		while (!done) {
			REQUIRE(action_set.has_value());
			auto const action = policy(action_set, model);
			std::tie(done, action_set) = dyn.step_dynamics(model, action);
		}
		REQUIRE(model.is_solved());
	}

	SECTION("Solve instance with single variables") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		while (!done) {
			REQUIRE(action_set.has_value());
			auto const action = action_set.value()[0];
			std::tie(done, action_set) = dyn.step_dynamics(model, {&action, 1});
		}
		REQUIRE(model.is_solved());
	}
}

TEST_CASE("BranchingGUBDynamics handles invalid inputs", "[dynamics]") {
	auto dyn = dynamics::BranchingGUBDynamics{};
	auto model = get_model();

	SECTION("Throw on invalid branching variable") {
		auto const [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
		REQUIRE(action_set.has_value());
		auto const action = model.lp_columns().size() + 1;
		REQUIRE_THROWS_AS(dyn.step_dynamics(model, {&action, 1}), std::invalid_argument);
	}
}
