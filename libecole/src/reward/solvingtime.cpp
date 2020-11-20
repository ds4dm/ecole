#include <chrono>
#include <ctime>

#include "ecole/reward/solvingtime.hpp"

namespace ecole::reward {

namespace {

auto wall_clock() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())
		.count();
}

}  // namespace

void SolvingTime::before_reset(scip::Model& /* model */) {
	if (wall) {
		solving_time_offset = wall_clock();
	} else {
		solving_time_offset = static_cast<long>(std::clock());
	}
}

Reward SolvingTime::extract(scip::Model& /* model */, bool /* done */) {
	double solving_time_diff = 0.0;
	long now = 0;

	if (wall) {
		static auto constexpr mus_per_seconds = 1000 * 1000;
		now = wall_clock();
		solving_time_diff = static_cast<double>(now - solving_time_offset) / mus_per_seconds;
	} else {
		now = static_cast<long>(std::clock());
		solving_time_diff = static_cast<double>(now - solving_time_offset) / CLOCKS_PER_SEC;
	}
	solving_time_offset = now;
	return solving_time_diff;
}

}  // namespace ecole::reward
