#pragma once

#include <catch2/catch.hpp>

#include "conftest.hpp"

namespace ecole::reward {

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
		reward_func.extract(solving_model, done);
	}
}

}  // namespace ecole::reward
