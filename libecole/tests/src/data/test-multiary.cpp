#include <functional>

#include <catch2/catch.hpp>

#include "ecole/data/multiary.hpp"

#include "conftest.hpp"
#include "data/mock-function.hpp"
#include "data/unit-tests.hpp"

using namespace ecole::data;

TEST_CASE("UnaryFunction unit tests", "[unit][data]") {
	// FIXME C++20: No type deduction for aliases
	ecole::data::unit_tests(MultiaryFunction{std::negate{}, IntDataFunc{}});
}

TEST_CASE("BinaryFunction unit tests", "[unit][data]") {
	// FIXME C++20: No type deduction for aliases
	ecole::data::unit_tests(MultiaryFunction{std::plus{}, IntDataFunc{}, IntDataFunc{}});
}

TEST_CASE("UnaryFunction negate the number", "[data]") {
	auto reward_func = MultiaryFunction{std::negate{}, IntDataFunc{}};
	auto model = get_model();

	reward_func.before_reset(model);
	advance_to_root_node(model);

	REQUIRE(reward_func.extract(model) < 0);
}

TEST_CASE("BinaryFunction substract two numbers", "[data]") {
	auto reward_func = MultiaryFunction{std::minus{}, IntDataFunc{}, IntDataFunc{}};
	auto model = get_model();

	reward_func.before_reset(model);
	advance_to_root_node(model);

	REQUIRE(reward_func.extract(model) == 0);
}
