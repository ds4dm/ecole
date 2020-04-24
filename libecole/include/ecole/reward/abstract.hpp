#pragma once

#include <memory>

#include "ecole/environment/state.hpp"

namespace ecole {
namespace reward {

using Reward = double;

/**
 * Abstract base class for all reward functions.
 *
 * Reward functions can be given to environments to parametrize what rewards are returned
 * at every transition.
 */
class RewardFunction {
public:
	virtual ~RewardFunction() = default;

	/**
	 * The method called by the environment on the initial state
	 *
	 * The method is called at the begining of every episode, and does nothing by default.
	 */
	virtual void reset(environment::State const& initial_state) { (void)initial_state; }

	/**
	 * The method called by the environment on every new state (after transitioning).
	 */
	virtual Reward obtain_reward(environment::State const& state, bool done = false) = 0;
};

}  // namespace reward
}  // namespace ecole
