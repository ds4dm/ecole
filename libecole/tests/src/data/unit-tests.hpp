#pragma once

#include <catch2/catch.hpp>

#include "conftest.hpp"

namespace ecole::data {

template <typename DataFunc> void unit_tests(DataFunc&& data_func) {
	auto solving_model = get_solving_model();

	SECTION("has default constructor") { DataFunc{}; }

	SECTION("reset, reset, and delete") {
		data_func.reset(solving_model);
		data_func.reset(solving_model);
	}

	SECTION("reset, obtain observation, and delete") {
		data_func.reset(solving_model);
		auto const done = GENERATE(true, false);
		data_func.extract(solving_model, done);
	}
}

}  // namespace ecole::data
