#pragma once

#include <memory>

#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace reward {

class IsDone : public RewardFunction {
public:
	Reward obtain_reward(environment::State const& state, bool done = false) override;
};

}  // namespace reward
}  // namespace ecole
