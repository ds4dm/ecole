#pragma once

#include <chrono>

#include "ecole/export.hpp"
#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class ECOLE_EXPORT SolvingTime {
public:
	SolvingTime(bool wall_ = false) noexcept : wall{wall_} {}

	ECOLE_EXPORT auto before_reset(scip::Model& model) -> void;
	ECOLE_EXPORT auto extract(scip::Model& model, bool done = false) -> Reward;

private:
	bool wall = false;
	std::chrono::nanoseconds solving_time_offset;
};

}  // namespace ecole::reward
