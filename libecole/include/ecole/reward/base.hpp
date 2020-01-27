#pragma once

#include <memory>

#include "ecole/scip/model.hpp"

namespace ecole {
namespace reward {

/**
 * Abstract base class for all reward functions.
 *
 * Reward functions can be given to environments to parametrize what rewards are returned
 * at every transition.
 */
class RewardFunction {
public:
	using reward_t = double;

	virtual ~RewardFunction() = default;
	virtual std::unique_ptr<RewardFunction> clone() const = 0;

	/**
	 * The method called by the environment at the begining of every episode (on the
	 * initial state).
	 */
	virtual void reset(scip::Model const& model);

	/**
	 * The method called by the environment on every new state (after transitioning).
	 */
	virtual reward_t get(scip::Model const& model, bool done = false) = 0;
};

}  // namespace reward
}  // namespace ecole
