#pragma once

#include <cstddef>

#include <xtensor/xtensor.hpp>

#include "ecole/default.hpp"
#include "ecole/dynamics/parts.hpp"
#include "ecole/export.hpp"

namespace ecole::dynamics {

class ECOLE_EXPORT BranchingDynamics : public DefaultSetDynamicsRandomState {
public:
	using Action = Defaultable<std::size_t>;
	using ActionSet = std::optional<xt::xtensor<std::size_t, 1>>;

	using DefaultSetDynamicsRandomState::set_dynamics_random_state;

	ECOLE_EXPORT BranchingDynamics(bool pseudo_candidates = false) noexcept;

	ECOLE_EXPORT auto reset_dynamics(scip::Model& model) const -> std::tuple<bool, ActionSet>;

	ECOLE_EXPORT auto step_dynamics(scip::Model& model, Action maybe_var_idx) const -> std::tuple<bool, ActionSet>;

private:
	bool pseudo_candidates;
};

}  // namespace ecole::dynamics
