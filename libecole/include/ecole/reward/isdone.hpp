#pragma once

#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace reward {

class IsDone : public RewardFunction {
public:
	Reward obtain_reward(scip::Model const& model, bool done = false) override;
};

}  // namespace reward
}  // namespace ecole
