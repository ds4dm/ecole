#pragma once

#include <random>
#include <string>
#include <tuple>

#include "ecole/environment/dynamics.hpp"
#include "ecole/reward/abstract.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace environment {

using Seed = typename RandomEngine::result_type;

using ecole::reward::Reward;
using Info = int;  // FIXME dummy type while the information is not implemented

/**
 * Abstract base class for all environments.
 *
 * Environments are the main abstraction exposed by Ecole.
 * They characterise the Markov Decision Process task to solve.
 * The interface to environments is meant to be close to that of
 * [OpenAi Gym](https://gym.openai.com/), with some differences nontheless due to the
 * requirements of Ecole.
 *
 * @tparam Action The type of the action used to transition from one state to another.
 * @tparam ActionSet A subset of Actions that are accepted by the environment.
 * @tparam Observation The type of the observation returned on every state.
 */
template <typename Action, typename ActionSet, typename Observation> class Environment {
public:
	virtual ~Environment() = default;

	/**
	 * Set the random seed for the environment, hence making its internals deterministic.
	 *
	 * Internally, the behavior of the environment uses a random number generator to
	 * change its behavior on every trajectroy (every call to @ref reset.
	 * Hence it is only required to seed the environment once.
	 *
	 * To get the same trajectory at every episode (provided the problem instance and
	 * sequence of action taken are also unchanged), one has to seed the environment before
	 * every call to reset.
	 */
	virtual void seed(Seed seed) = 0;

	/**
	 * Reset the environment to the initial state on the given problem instance.
	 *
	 * Takes as input a filename or loaded model.
	 *
	 * @return An observation of the new state, or nothing on terminal states.
	 * @return An subset of actions accepted on the next transition (call to @ref step).
	 * @return A boolean flag indicating whether the state is terminal.
	 * @post Unless the (initial) state is also terminal, transitioning (using step) is
	 *       possible.
	 */
	virtual std::tuple<Observation, ActionSet, bool> reset(scip::Model&& model) = 0;
	virtual std::tuple<Observation, ActionSet, bool> reset(scip::Model const& model) = 0;
	virtual std::tuple<Observation, ActionSet, bool> reset(std::string const& filename) = 0;

	/**
	 * Transition from one state to another.
	 *
	 * Take an action on the previously observed state and transition to a new state.
	 *
	 * @param action
	 * @return An observation of the new state, or nothing on terminal states.
	 * @return An subset of actions accepted on the next transition (call to @ref step).
	 * @return A scalar reward from the signal to maximize.
	 * @return A boolean flag indicating whether the state is terminal.
	 * @return Any additional information about the transition.
	 * @pre A call to reset must have been done prior to transitioning.
	 * @pre The envrionment must not be on a terminal state, or have thrown an exception.
	 *      In such cases, a call to reset must be perform before continuing.
	 */
	virtual std::tuple<Observation, ActionSet, Reward, bool, Info>
	step(Action const& action) = 0;
};

}  // namespace environment
}  // namespace ecole
