#include <chrono>
#include <ctime>

#include "ecole/reward/solvingtime.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::reward {

const int MUS_IN_SECONDS = 1000000;

void SolvingTime::before_reset(scip::Model& /* model */) {

	if (wall) {
		solving_time_offset =
			std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())
				.count();
	} else {
		solving_time_offset = std::clock();
	}
}

Reward SolvingTime::extract(scip::Model& /* model */, bool /* done */) {
	double solving_time_diff;
	long now;

	if (wall) {
		now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())
						.count();
		solving_time_diff = double(now - solving_time_offset) / MUS_IN_SECONDS;
	} else {
		now = std::clock();
		solving_time_diff = double(now - solving_time_offset) / CLOCKS_PER_SEC;
	}
	solving_time_offset = now;
	return solving_time_diff;
}

}  // namespace ecole::reward
