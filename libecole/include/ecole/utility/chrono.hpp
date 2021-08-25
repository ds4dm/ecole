#pragma once

#include <chrono>

#include "ecole/export.hpp"

namespace ecole::utility {

/**
 * A CPU usage clock.
 *
 * Measure time the CPU spent processing the program’s instructions.
 * This count both the System (kernel) and User CPU time.
 * The time spent waiting for other things to complete (like I/O operations) is not included in the CPU time.
 *
 * The implementation uses OS dependent functionality.
 */
class ECOLE_EXPORT cpu_clock {
public:
	using duration = std::chrono::nanoseconds;
	using rep = duration::rep;
	using period = duration::period;
	using time_point = std::chrono::time_point<cpu_clock>;
	static bool constexpr is_steady = true;

	ECOLE_EXPORT static auto now() -> time_point;
};

}  // namespace ecole::utility
