#pragma once

#include <exception>

namespace ecole::utility {

/** To be used in part of the code that are known to be unreachable (e.g. the default case in an enum `switch`). */
[[noreturn]] inline void unreachable() {
	std::terminate();
}

}  // namespace ecole::utility
