#pragma once

#include <cstddef>

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/environment/dynamics.hpp"
#include "ecole/environment/state.hpp"

namespace ecole {
namespace environment {

class BranchingDynamics :
	public EnvironmentDynamics<
		std::size_t,
		nonstd::optional<xt::xtensor<std::size_t, 1>>,
		State> {
public:
	using ActionSet = nonstd::optional<xt::xtensor<std::size_t, 1>>;

	bool pseudo_candidates;

	BranchingDynamics(bool pseudo_candidates = false) noexcept;

	std::tuple<bool, ActionSet> reset_dynamics(State& initial_state) override;

	std::tuple<bool, ActionSet>
	step_dynamics(State& state, std::size_t const& action) override;
};

}  // namespace environment
}  // namespace ecole
