#pragma once

#include <catch2/catch.hpp>

#include "conftest.hpp"

namespace ecole::reward {

template <typename RewardFunc> void unit_tests(RewardFunc&& reward_func) {
	auto done = GENERATE(true, false);
	auto solving_model = get_solving_model();

	SECTION("has default constructor") { RewardFunc{}; }

	SECTION("before_reset, before_reset, and delete") {
		reward_func.before_reset(solving_model);
		reward_func.before_reset(solving_model);
	}

	SECTION("before_reset, obtain reward, and delete") {
		reward_func.before_reset(solving_model);
		reward_func.extract(solving_model, done);
	}
}

}  // namespace ecole::reward
