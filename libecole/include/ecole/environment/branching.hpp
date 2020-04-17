#pragma once

#include <cstddef>

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

class BranchingDynamics {
public:
	using Action = std::size_t;
	using State = ReverseControlState;

	bool reset_state(State& initial_state);
	bool step_state(State& state, std::size_t const& action);
};

template <typename... EnvTypes>
using Branching = DefaultEnvironment<BranchingDynamics, EnvTypes...>;

}  // namespace environment
}  // namespace ecole
