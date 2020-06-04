#pragma once

#include <memory>

#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace reward {

class LpIterations : public RewardFunction {
public:
	void reset(scip::Model const& model) override;
	Reward obtain_reward(scip::Model const& model, bool done = false) override;

private:
	scip::long_int last_lp_iter = 0;
};

}  // namespace reward
}  // namespace ecole
