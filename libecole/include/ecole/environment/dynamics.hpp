#pragma once

#include <tuple>

namespace ecole {
namespace environment {

/**
 * Abstract class for environment Dynamics.
 *
 * A subclass defines the dynamics of the environment, that is the initial probability
 * distribution and state transition.
 * In other words, it defines an environment wihtout observations or rewards.
 * This class is used by @ref EnvironmentComposer to create the final environment with
 * state functions.
 *
 * @tparam Action The type of action recived by the environment.
 * @tparam ActionSet The type used to indicate what actions are accepted on the next
 *         transtion.
 * @tparam State The state defines the state of the environment and is shared with
 *         state functions.
 *         It contains at least the SCIP @ref scip::Model
 */
template <typename Action, typename ActionSet, typename State> class EnvironmentDynamics {
public:
	virtual ~EnvironmentDynamics() = default;

	/**
	 * Reset the State to a new initial state.
	 *
	 * This method called by the environment on @ref Environment::reset.
	 */
	virtual std::tuple<bool, ActionSet> reset_dynamics(State& init_state) = 0;

	/**
	 * Transition the State.
	 *
	 * This method called by the environment on @ref Environment::step.
	 */
	virtual std::tuple<bool, ActionSet>
	step_dynamics(State& state, Action const& action) = 0;
};

}  // namespace environment
}  // namespace ecole
