#pragma once

#include <catch2/catch.hpp>

#include "conftest.hpp"

namespace ecole::observation {

template <typename ObsFunc> void unit_tests(ObsFunc&& obs_func) {
	auto solving_model = get_solving_model();

	SECTION("has default constructor") { ObsFunc{}; }

	SECTION("reset, reset, and delete") {
		obs_func.reset(solving_model);
		obs_func.reset(solving_model);
	}

	SECTION("reset, obtain observation, and delete") {
		obs_func.reset(solving_model);
		auto const done = GENERATE(true, false);
		obs_func.extract(solving_model, done);
	}
}

}  // namespace ecole::observation
