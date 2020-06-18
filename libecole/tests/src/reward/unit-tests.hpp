#pragma once

#include <catch2/catch.hpp>

#include "conftest.hpp"

template <typename RewardFunc> void unit_tests(RewardFunc&& reward_func) {
	auto solving_model = get_solving_model();

	SECTION("has default constructor") { RewardFunc{}; }

	SECTION("reset, reset, and delete") {
		reward_func.reset(solving_model);
		reward_func.reset(solving_model);
	}

	SECTION("reset, obtain reward, and delete") {
		reward_func.reset(solving_model);
		reward_func.obtain_reward(solving_model);
	}
}
