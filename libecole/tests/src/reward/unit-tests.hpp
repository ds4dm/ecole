#pragma once

#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/traits.hpp"

#include "conftest.hpp"
#include "data/unit-tests.hpp"

namespace ecole::reward {

template <typename RewardFunc> void unit_tests(RewardFunc&& reward_func) {
	SECTION("Interface is valid") { STATIC_REQUIRE(trait::is_reward_function_v<RewardFunc>); };

	SECTION("Function has default constructor") { STATIC_REQUIRE(std::is_default_constructible_v<RewardFunc>); }

	data::unit_tests(std::forward<RewardFunc>(reward_func));
}

}  // namespace ecole::reward
