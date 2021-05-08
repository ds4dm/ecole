#pragma once

#include <map>
#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/dynamics.hpp"

namespace ecole::dynamics {

/**
 * An array of variable identifiers in the transformed problem.
 */
using VarIds = xt::xtensor<std::size_t, 1>;

/**
 * A dictionnary of variable identifiers to variable values.
 */
using VarIdVals = std::map<std::size_t, SCIP_Real>;

class PrimalSearchDynamics : public EnvironmentDynamics<VarIdVals, std::optional<VarIds>> {
public:
	using ActionSet = std::optional<VarIds>;
	using Action = VarIdVals;

	int trials_per_node;
	int depth_freq;
	int depth_start;
	int depth_stop;

	PrimalSearchDynamics(int trials_per_node = 1, int depth_freq = 1, int depth_start = 0, int depth_stop = -1) noexcept(
		false);

	std::tuple<bool, ActionSet> reset_dynamics(scip::Model& model) override;

	std::tuple<bool, ActionSet> step_dynamics(scip::Model& model, Action const& action) override;
};

}  // namespace ecole::dynamics
