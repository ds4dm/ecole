#include <catch2/catch.hpp>

#include "ecole/data/constant.hpp"

#include "conftest.hpp"
#include "data/unit-tests.hpp"

using namespace ecole;

TEST_CASE("Constant reward unit tests", "[unit][data]") {
	constexpr auto some_constant = 3.0;
	data::unit_tests(data::ConstantFunction{some_constant});
}

TEST_CASE("Constant reward always return the same value", "[data]") {
	auto const done = GENERATE(true, false);
	auto const constant = GENERATE(-1., 0., 55);
	auto data_func = data::ConstantFunction{constant};
	auto model = get_model();

	data_func.before_reset(model);
	advance_to_root_node(model);

	REQUIRE(data_func.extract(model, done) == constant);

	SECTION("On successive calls") { REQUIRE(data_func.extract(model, done) == constant); }
}
