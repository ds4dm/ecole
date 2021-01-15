#pragma once

#include <catch2/catch.hpp>

#include "conftest.hpp"

namespace ecole::reward {

template <typename RewardFunc> void unit_tests(RewardFunc&& reward_func) {
	auto done = GENERATE(true, false);

	SECTION("has default constructor") { RewardFunc{}; }

	SECTION("before_reset, before_reset, and delete") {
		auto model = get_model();
		reward_func.before_reset(model);
		reward_func.before_reset(model);
	}

	SECTION("before_reset, obtain reward, and delete") {
		auto model = get_model();
		reward_func.before_reset(model);
		advance_to_root_node(model);
		reward_func.extract(model, done);
	}
}

}  // namespace ecole::reward
