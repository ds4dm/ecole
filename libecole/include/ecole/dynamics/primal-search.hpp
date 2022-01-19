#pragma once

#include <cstddef>
#include <optional>
#include <utility>

#include <nonstd/span.hpp>
#include <scip/def.h>
#include <scip/type_result.h>
#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/parts.hpp"
#include "ecole/export.hpp"

namespace ecole::dynamics {

class ECOLE_EXPORT PrimalSearchDynamics : public DefaultSetDynamicsRandomState {
public:
	/** An array of variable identifiers in the transformed problem. */
	using ActionSet = std::optional<xt::xtensor<std::size_t, 1>>;
	/** A tuple of variable identifiers and variable values. */
	using Action = std::pair<nonstd::span<std::size_t const>, nonstd::span<SCIP_Real const>>;

	ECOLE_EXPORT
	PrimalSearchDynamics(int trials_per_node = 1, int depth_freq = 1, int depth_start = 0, int depth_stop = -1);

	using DefaultSetDynamicsRandomState::set_dynamics_random_state;

	ECOLE_EXPORT auto reset_dynamics(scip::Model& model) const -> std::tuple<bool, ActionSet>;

	ECOLE_EXPORT auto step_dynamics(scip::Model& model, Action action) -> std::tuple<bool, ActionSet>;

private:
	int trials_per_node;
	int depth_freq;
	int depth_start;
	int depth_stop;

	unsigned int trials_spent = 0;        // to keep track of the number of trials during each search
	SCIP_RESULT result = SCIP_DIDNOTRUN;  // the final result of each search (several trials)
};

}  // namespace ecole::dynamics
