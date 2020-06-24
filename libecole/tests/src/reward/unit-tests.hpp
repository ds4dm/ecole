#pragma once

#include <catch2/catch.hpp>

#include "conftest.hpp"

namespace ecole {
namespace reward {

template <typename RewardFunc> void unit_tests(RewardFunc&& reward_func) {
	auto done = GENERATE(true, false);
	auto solving_model = get_solving_model();

	SECTION("has default constructor") { RewardFunc{}; }

	SECTION("reset, reset, and delete") {
		reward_func.reset(solving_model);
		reward_func.reset(solving_model);
	}

	SECTION("reset, obtain reward, and delete") {
		reward_func.reset(solving_model);
		reward_func.obtain_reward(solving_model, done);
	}
}

}  // namespace reward
}  // namespace ecole
