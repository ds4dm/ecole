#pragma once

#include <utility>

#include "data/unit-tests.hpp"

namespace ecole::observation {

template <typename ObsFunc> void unit_tests(ObsFunc&& obs_func) {
	data::unit_tests(std::forward<ObsFunc>(obs_func));
}

}  // namespace ecole::observation
