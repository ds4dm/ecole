#pragma once

#include <chrono>

#include "ecole/export.hpp"
#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class ECOLE_EXPORT SolvingTime : public RewardFunction {
public:
	SolvingTime(bool wall_ = false) noexcept : wall{wall_} {}

	ECOLE_EXPORT void before_reset(scip::Model& model) override;
	ECOLE_EXPORT Reward extract(scip::Model& model, bool done = false) override;

private:
	bool wall = false;
	std::chrono::nanoseconds solving_time_offset;
};

}  // namespace ecole::reward
