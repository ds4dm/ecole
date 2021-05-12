#include <catch2/catch.hpp>

#include "ecole/data/timed.hpp"

#include "conftest.hpp"
#include "data/mock-function.hpp"
#include "data/unit-tests.hpp"

using namespace ecole;

TEST_CASE("Data TimedFunction unit tests", "[unit][data]") {
	bool const wall = GENERATE(true, false);
	data::unit_tests(data::TimedFunction<data::IntDataFunc>{wall});
}

TEST_CASE("Timed data function is positive", "[data]") {
	bool const wall = GENERATE(true, false);
	auto timed_func = data::TimedFunction<data::IntDataFunc>{wall};
	auto model = get_model();

	timed_func.before_reset(model);
	advance_to_root_node(model);
	auto const time = timed_func.extract(model, false);
	REQUIRE(time >= 0.);
}
