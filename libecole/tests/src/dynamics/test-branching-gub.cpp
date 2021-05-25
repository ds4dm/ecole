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
struct MultiBrancingPolicy {
	std::size_t n_multi = 0;

	auto operator()(trait::action_set_of_t<dynamics::BranchingGUBDynamics> const& action_set, scip::Model& model)
		-> std::vector<std::size_t> {
		auto as = action_set.value();
		auto rng = RandomEngine{as[0] + as.size()};
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
	auto policy = MultiBrancingPolicy{};
	dynamics::unit_tests(dynamics::BranchingGUBDynamics{}, policy);
}
