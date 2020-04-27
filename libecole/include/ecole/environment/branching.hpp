#pragma once

#include <cstddef>

#include <xtensor/xtensor.hpp>

#include "ecole/environment/default.hpp"
#include "ecole/environment/state.hpp"
#include "ecole/utility/reverse-control.hpp"

namespace ecole {
namespace environment {

class ReverseControlState : public State {
public:
	ReverseControlState() = default;
	explicit ReverseControlState(scip::Model&& model);
	explicit ReverseControlState(scip::Model const& model);
	ReverseControlState(ReverseControlState const&) = delete;
	ReverseControlState(ReverseControlState&&);
	ReverseControlState& operator=(ReverseControlState const&) = delete;
	ReverseControlState& operator=(ReverseControlState&&);

	std::unique_ptr<utility::Controller> controller = nullptr;
};

class BranchingDynamics :
	public EnvironmentDynamics<
		std::size_t,
		xt::xtensor<std::size_t, 1>,
		ReverseControlState> {
public:
	using Action = std::size_t;
	using ActionSet = xt::xtensor<size_t, 1>;
	using State = ReverseControlState;

	std::tuple<bool, xt::xtensor<std::size_t, 1>>
	reset_dynamics(State& initial_state) override;

	std::tuple<bool, xt::xtensor<std::size_t, 1>>
	step_dynamics(State& state, std::size_t const& action) override;
};

template <typename... EnvTypes>
using Branching = EnvironmentComposer<BranchingDynamics, EnvTypes...>;

}  // namespace environment
}  // namespace ecole
