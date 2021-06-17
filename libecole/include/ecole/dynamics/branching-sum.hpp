#pragma once

#include <cstddef>
#include <optional>

#include <nonstd/span.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/dynamics.hpp"

namespace ecole::dynamics {

class BranchingSumDynamics :
	public EnvironmentDynamics<nonstd::span<std::size_t const>, std::optional<xt::xtensor<std::size_t, 1>>> {
public:
	using Action = nonstd::span<std::size_t const>;
	using ActionSet = std::optional<xt::xtensor<std::size_t, 1>>;

	std::tuple<bool, ActionSet> reset_dynamics(scip::Model& model) override;

	std::tuple<bool, ActionSet> step_dynamics(scip::Model& model, Action const& var_indices) override;
};

}  // namespace ecole::dynamics
