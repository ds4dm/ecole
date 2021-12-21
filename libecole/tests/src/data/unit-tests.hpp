#pragma once

#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/traits.hpp"

#include "conftest.hpp"

namespace ecole::data {

template <typename DataFunc> void unit_tests(DataFunc&& data_func) {
	SECTION("Interface is valid") { STATIC_REQUIRE(trait::is_data_function_v<DataFunc>); };

	SECTION("has default constructor") { STATIC_REQUIRE(std::is_default_constructible_v<DataFunc>); }

	SECTION("before_reset, before_reset, and delete") {
		auto model = get_model();
		data_func.before_reset(model);
		data_func.before_reset(model);
	}

	SECTION("before_reset, obtain observation, and delete") {
		auto model = get_model();
		data_func.before_reset(model);
		advance_to_stage(model, SCIP_STAGE_SOLVING);
		auto const done = GENERATE(true, false);
		data_func.extract(model, done);
	}
}

}  // namespace ecole::data
