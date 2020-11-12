#include <catch2/catch.hpp>

#include "ecole/reward/constant.hpp"
#include "ecole/reward/unary.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

// FIXME C++20: Captureless lambda are now default constructible.
struct Increment {
	auto operator()(double reward) const noexcept { return reward + 1.; }
};

constexpr auto some_constant = 3.0;
constexpr auto increment = Increment{};

TEST_CASE("Unary reward unit tests", "[unit][reward]") {
	reward::unit_tests(reward::UnaryFunction{reward::Constant{some_constant}, increment});
}

TEST_CASE("Unary reward always return the same value", "[reward]") {
	auto reward_func = reward::UnaryFunction{reward::Constant{some_constant}, increment};
	auto model = get_solving_model();

	reward_func.reset(model);

	REQUIRE(reward_func.obtain_reward(model) == increment(some_constant));
}
