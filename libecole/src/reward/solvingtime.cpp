#include <chrono>

#include "ecole/reward/solvingtime.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::reward {

namespace {

auto time_now(bool wall) -> std::chrono::nanoseconds {
	if (wall) {
		return std::chrono::steady_clock::now().time_since_epoch();
	}
	return utility::cpu_clock::now().time_since_epoch();
}

}  // namespace

void SolvingTime::before_reset(scip::Model& /* model */) {
	solving_time_offset = time_now(wall);
}

Reward SolvingTime::extract(scip::Model& /* model */, bool /* done */) {
	auto const now = time_now(wall);
	auto const solving_time_diff = static_cast<double>((now - solving_time_offset).count());
	solving_time_offset = now;
	return solving_time_diff;
}

}  // namespace ecole::reward
