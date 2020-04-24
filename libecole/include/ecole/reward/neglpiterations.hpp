#pragma once

#include <memory>

#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace reward {

class NegLPIterations : public RewardFunction {
public:
	void reset(environment::State const& initial_state) override;
	Reward obtain_reward(environment::State const& state, bool done = false) override;

private:
	scip::long_int last_lp_iter = 0;
};

}  // namespace reward
}  // namespace ecole
