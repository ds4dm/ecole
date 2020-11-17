#include <catch2/catch.hpp>

#include "ecole/data/nothing.hpp"

#include "conftest.hpp"
#include "data/unit-tests.hpp"

using namespace ecole::data;

TEST_CASE("Nothing unit tests", "[unit][data]") {
	unit_tests(Nothing{});
}

TEST_CASE("Nothing return None as data", "[data]") {
	auto const done = GENERATE(true, false);
	auto data_func = Nothing{};
	auto model = get_model();
	data_func.reset(model);

	REQUIRE(data_func.extract(model, done) == ecole::None);
}
