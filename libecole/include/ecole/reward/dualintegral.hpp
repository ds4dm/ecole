#pragma once

#include <chrono>

#include "ecole/reward/abstract.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::reward {

class DualIntegral : public RewardFunction {
public:
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	scip::real last_dual_bound = 0.0;
	std::chrono::nanoseconds solving_time_offset;
  bool wall = false;
};

}  // namespace ecole::reward
