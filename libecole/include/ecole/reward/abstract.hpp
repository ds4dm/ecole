#pragma once

namespace ecole {

namespace scip {
class Model;
}

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
	virtual void reset(scip::Model& /* model */) {}

	/**
	 * The method called by the environment on every new state (after transitioning).
	 */
	virtual Reward obtain_reward(scip::Model& model, bool done = false) = 0;
};

}  // namespace reward
}  // namespace ecole
