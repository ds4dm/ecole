#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/data/tuple.hpp"

#include "conftest.hpp"
#include "data/mock-function.hpp"
#include "data/unit-tests.hpp"

using namespace ecole::data;

TEST_CASE("Data TupleFunction unit tests", "[unit][data]") {
	ecole::data::unit_tests(TupleFunction{IntDataFunc{}, DoubleDataFunc{}});
}

TEST_CASE("Combine data functions into a tuple", "[data]") {
	auto data_func = TupleFunction{IntDataFunc{0}, DoubleDataFunc{1}};
	auto model = get_model();

	data_func.before_reset(model);
	advance_to_root_node(model);
	auto const data = data_func.extract(model, false);
	STATIC_REQUIRE(std::is_same_v<std::remove_const_t<decltype(data)>, std::tuple<int, double>>);
	REQUIRE(std::get<0>(data) == 1);
	REQUIRE(std::get<1>(data) == 2.0);  // NOLINT(readability-magic-numbers)
}
