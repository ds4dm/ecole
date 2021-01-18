#pragma once

#include <chrono>

#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class SolvingTime : public RewardFunction {
public:
	SolvingTime(bool wall_ = false) noexcept : wall{wall_} {}

	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	bool wall = false;
	std::chrono::nanoseconds solving_time_offset;
};

}  // namespace ecole::reward
