#include <cerrno>
#include <ctime>
#include <system_error>

#include "ecole/utility/chrono.hpp"

namespace ecole::utility {

/**
 * There is no standard way to get CPU time.
 *
 * The following implementation is inspired from
 *  - https://levelup.gitconnected.com/8-ways-to-measure-execution-time-in-c-c-48634458d0f9
 *  - https://stackoverflow.com/a/12480485/5862073
 *  - Google Benchmark implementation
 *    https://github.com/google/benchmark/blob/8df87f6c879cbcabd17c5cfcec7b89687df36953/src/timers.cc#L110
 */
auto cpu_clock::now() -> time_point {
	// Using clock_gettime is not standard but POSIX. It has nanoseconds resolution.
	// It works on Linux and MacOS >= 10.12
	struct timespec spec;
	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &spec) != 0) {
		throw std::system_error{{errno, std::generic_category()}};
	}
	return time_point{std::chrono::seconds{spec.tv_sec} + std::chrono::nanoseconds{spec.tv_nsec}};
}

}  // namespace ecole::utility
