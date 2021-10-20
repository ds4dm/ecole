#pragma once

#include <cstddef>
#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/dynamics.hpp"
#include "ecole/export.hpp"

namespace ecole::dynamics {

class ECOLE_EXPORT BranchingDynamics :
	public EnvironmentDynamics<std::size_t, std::optional<xt::xtensor<std::size_t, 1>>> {
public:
	using ActionSet = std::optional<xt::xtensor<std::size_t, 1>>;

	ECOLE_EXPORT BranchingDynamics(bool pseudo_candidates = false) noexcept;

	ECOLE_EXPORT auto reset_dynamics(scip::Model& model) -> std::tuple<bool, ActionSet> override;

	ECOLE_EXPORT auto step_dynamics(scip::Model& model, std::size_t const& var_idx)
		-> std::tuple<bool, ActionSet> override;

private:
	bool pseudo_candidates;
};

}  // namespace ecole::dynamics
