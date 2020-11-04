#pragma once

#include "ecole/reward/abstract.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::reward {

class SolvingTime : public RewardFunction {
public:
	SolvingTime(bool wall_ = false) : wall(wall_) {}
	void reset(scip::Model& model) override;
	Reward obtain_reward(scip::Model& model, bool done = false) override;

private:
	bool const wall = false;
	double solving_time_offset = 0;
};

}  // namespace ecole::reward
