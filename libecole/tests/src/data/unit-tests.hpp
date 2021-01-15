#pragma once

#include <catch2/catch.hpp>

#include "conftest.hpp"

namespace ecole::data {

template <typename DataFunc> void unit_tests(DataFunc&& data_func) {
	SECTION("has default constructor") { DataFunc{}; }

	SECTION("before_reset, before_reset, and delete") {
		auto model = get_model();
		data_func.before_reset(model);
		data_func.before_reset(model);
	}

	SECTION("before_reset, obtain observation, and delete") {
		auto model = get_model();
		data_func.before_reset(model);
		advance_to_root_node(model);
		auto const done = GENERATE(true, false);
		data_func.extract(model, done);
	}
}

}  // namespace ecole::data
