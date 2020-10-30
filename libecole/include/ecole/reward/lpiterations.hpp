#pragma once

#include "ecole/reward/abstract.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::reward {

class LpIterations : public RewardFunction {
public:
	void reset(scip::Model& model) override;
	Reward obtain_reward(scip::Model& model, bool done = false) override;

private:
	scip::long_int last_lp_iter = 0;
};

}  // namespace ecole::reward
