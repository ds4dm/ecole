#pragma once

#include <cstddef>

#include "ecole/environment/default.hpp"
#include "ecole/environment/state.hpp"
#include "ecole/utility/reverse-control.hpp"

namespace ecole {
namespace environment {

class BranchingDynamics {
public:
	using Action = std::size_t;
	using State = environment::State;

	bool reset_state(State& initial_state);
	bool step_state(State& state, std::size_t const& action);
	void del_state(State&);  // FIXME issue #24

private:
	std::unique_ptr<utility::Controller> controller = nullptr;
};

template <typename... EnvTypes>
using Branching = DefaultEnvironment<BranchingDynamics, EnvTypes...>;

}  // namespace environment
}  // namespace ecole
