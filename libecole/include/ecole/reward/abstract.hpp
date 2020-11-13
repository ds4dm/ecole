#pragma once

#include "ecole/data/abstract.hpp"

namespace ecole::reward {

using Reward = double;

/**
 * Abstract base class for all reward functions.
 *
 * Reward functions can be given to environments to parametrize what rewards are returned
 * at every transition.
 * Reward function are data extraction function that extrsct a reward
 */
using RewardFunction = data::DataFunction<Reward>;

}  // namespace ecole::reward
