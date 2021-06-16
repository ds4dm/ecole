#pragma once

#include <catch2/catch.hpp>

#include "conftest.hpp"

namespace ecole::reward {

template <typename RewardFunc> void unit_tests(RewardFunc&& reward_func) {
	auto done = GENERATE(true, false);

	SECTION("has default constructor") { RewardFunc{}; }

	SECTION("before_reset, before_reset, and delete") {
		auto model1 = get_model();
		reward_func.before_reset(model1);
		auto model2 = get_model();
		reward_func.before_reset(model2);
	}

	SECTION("before_reset, obtain reward, and delete") {
		auto model = get_model();
		reward_func.before_reset(model);
		advance_to_stage(model, SCIP_STAGE_SOLVING);
		reward_func.extract(model, done);
	}
}

}  // namespace ecole::reward
