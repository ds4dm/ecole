#pragma once

#include <cstddef>
#include <optional>
#include <utility>

#include <nonstd/span.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/dynamics.hpp"

namespace ecole::dynamics {

/**
 * An array of variable identifiers in the transformed problem.
 */
using VarIds = xt::xtensor<std::size_t, 1>;

/**
 * A tuple of variable identifiers and variable values.
 */
using VarIdsVals = std::pair<nonstd::span<std::size_t const>, nonstd::span<SCIP_Real const>>;

class PrimalSearchDynamics : public EnvironmentDynamics<VarIdsVals, std::optional<VarIds>> {
public:
	using Action = VarIdsVals;
	using ActionSet = std::optional<VarIds>;

	PrimalSearchDynamics(int trials_per_node = 1, int depth_freq = 1, int depth_start = 0, int depth_stop = -1);

	std::tuple<bool, ActionSet> reset_dynamics(scip::Model& model) override;

	std::tuple<bool, ActionSet> step_dynamics(scip::Model& model, Action const& action) override;

private:
	int trials_per_node;
	int depth_freq;
	int depth_start;
	int depth_stop;

	unsigned int trials_spent = 0;        // to keep track of the number of trials during each search
	SCIP_HEUR* heur = nullptr;            // to tell SCIP where primal solutions come from
	SCIP_RESULT result = SCIP_DIDNOTRUN;  // the final result of each search (several trials)
};

}  // namespace ecole::dynamics
