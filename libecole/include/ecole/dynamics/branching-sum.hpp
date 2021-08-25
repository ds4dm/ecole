#pragma once

#include <cstddef>
#include <optional>

#include <nonstd/span.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/dynamics.hpp"
#include "ecole/export.hpp"

namespace ecole::dynamics {

class ECOLE_EXPORT BranchingSumDynamics :
	public EnvironmentDynamics<nonstd::span<std::size_t const>, std::optional<xt::xtensor<std::size_t, 1>>> {
public:
	using Action = nonstd::span<std::size_t const>;
	using ActionSet = std::optional<xt::xtensor<std::size_t, 1>>;

	ECOLE_EXPORT auto reset_dynamics(scip::Model& model) -> std::tuple<bool, ActionSet> override;

	ECOLE_EXPORT auto step_dynamics(scip::Model& model, Action const& var_indices)
		-> std::tuple<bool, ActionSet> override;
};

}  // namespace ecole::dynamics
