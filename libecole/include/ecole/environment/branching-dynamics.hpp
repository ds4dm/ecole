#pragma once

#include <cstddef>

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/environment/dynamics.hpp"

namespace ecole::environment {

class BranchingDynamics : public EnvironmentDynamics<std::size_t, nonstd::optional<xt::xtensor<std::size_t, 1>>> {
public:
	using ActionSet = nonstd::optional<xt::xtensor<std::size_t, 1>>;

	bool pseudo_candidates;

	BranchingDynamics(bool pseudo_candidates = false) noexcept;

	std::tuple<bool, ActionSet> reset_dynamics(scip::Model& model) override;

	std::tuple<bool, ActionSet> step_dynamics(scip::Model& model, std::size_t const& action) override;
};

}  // namespace ecole::environment
