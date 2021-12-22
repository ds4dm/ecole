#pragma once

#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/traits.hpp"

#include "conftest.hpp"

namespace ecole::reward {

template <typename RewardFunc> void unit_tests(RewardFunc&& reward_func) {
	auto done = GENERATE(true, false);

	SECTION("Interface is valid") { STATIC_REQUIRE(trait::is_reward_function_v<RewardFunc>); };

	SECTION("Function has default constructor") { STATIC_REQUIRE(std::is_default_constructible_v<RewardFunc>); }

	SECTION("Function before_reset, before_reset, and delete") {
		auto model1 = get_model();
		reward_func.before_reset(model1);
		auto model2 = get_model();
		reward_func.before_reset(model2);
	}

	SECTION("before_reset, obtain reward, and delete") {
		auto model = get_model();
		reward_func.before_reset(model);
		advance_to_stage(model, SCIP_STAGE_SOLVING);
		[[maybe_unused]] auto const reward = reward_func.extract(model, done);
	}
}

}  // namespace ecole::reward
