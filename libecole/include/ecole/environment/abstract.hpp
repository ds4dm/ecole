#pragma once

#include <string>
#include <tuple>

#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace environment {

using ecole::reward::Reward;
using Seed = int;
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
 * @tparam Action_ The type of the action accepted to transition from one state to
 * another.
 * @tparam Observation_ The type of the observation returned on every state.
 */
template <typename Action_, typename Observation_> class Environment {
public:
	using Action = Action_;
	using Observation = Observation_;

	virtual ~Environment() = default;

	/**
	 * Set the random seed for the environment, hence making its internals deterministic.
	 *
	 * The seed is deteministically changed at every new episode (every call to reset) to
	 * avoid overfitting on a single seed.
	 * To get the same trajectory at every episode (provided the problem instance and
	 * sequence of action taken are also unchanged), one has to seed the environment before
	 * every call to reset.
	 */
	virtual void seed(Seed seed) = 0;

	/**
	 * Get the current random seed.
	 */
	virtual Seed seed() const = 0;

	/**
	 * Reset the environment to the initial state on the given problem instance.
	 *
	 * @return An observation of the new state.
	 * @return A boolean flag indicating whether the state is terminal.
	 * @post Unless the (initial) state is also terminal, transitioning (using step) is
	 *       possible.
	 */
	virtual std::tuple<Observation, bool> reset(std::string const& filename) = 0;

	/**
	 * Transition from one state to another.
	 *
	 * Take an action on the previously observed state and transition to a new state.
	 *
	 * @param action
	 * @return An observation of the new state.
	 * @return A scalar reward from the signal to maximize.
	 * @return A boolean flag indicating whether the state is terminal.
	 * @return Any additional information about the transition.
	 * @pre A call to reset must have been done prior to transitioning.
	 * @pre The envrionment must not be on a terminal state, or have thrown an exception.
	 *      In such cases, a call to reset must be perform before continuing.
	 */
	virtual std::tuple<Observation, Reward, bool, Info> step(Action const& action) = 0;
};

}  // namespace environment
}  // namespace ecole
