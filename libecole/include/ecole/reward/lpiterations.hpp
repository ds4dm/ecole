#pragma once

#include <cstdint>

#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class LpIterations : public RewardFunction {
public:
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	std::uint64_t last_lp_iter = 0;
};

}  // namespace ecole::reward
