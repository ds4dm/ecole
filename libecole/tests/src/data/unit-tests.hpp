#pragma once

#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/traits.hpp"

#include "conftest.hpp"

namespace ecole::data {

template <typename DataFunc> void unit_tests(DataFunc&& data_func) {
	SECTION("Interface is valid") { STATIC_REQUIRE(trait::is_data_function_v<DataFunc>); };

	SECTION("Has valid constructors") {
		if constexpr (std::is_default_constructible_v<DataFunc>) {
			[[maybe_unused]] auto const func = DataFunc{};  // NOLINT
		}
		if constexpr (std::is_copy_constructible_v<DataFunc>) {
			[[maybe_unused]] auto const func = DataFunc{data_func};  // NOLINT
		}
		if constexpr (std::is_copy_assignable_v<DataFunc>) {
			[[maybe_unused]] auto const& func = data_func;  // NOLINT
		}
	}

	SECTION("Function before_reset, before_reset, and delete") {
		auto model1 = get_model();
		data_func.before_reset(model1);
		auto model2 = get_model();
		data_func.before_reset(model2);
	}

	SECTION("Function before_reset, obtain data, and delete") {
		auto model = get_model();
		data_func.before_reset(model);
		advance_to_stage(model, SCIP_STAGE_SOLVING);
		auto const done = GENERATE(true, false);
		[[maybe_unused]] auto const data = data_func.extract(model, done);
	}
}

}  // namespace ecole::data
