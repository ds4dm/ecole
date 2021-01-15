#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/data/vector.hpp"

#include "conftest.hpp"
#include "data/mock-function.hpp"
#include "data/unit-tests.hpp"

using namespace ecole::data;

TEST_CASE("Data VectorFunction unit tests", "[unit][data]") {
	ecole::data::unit_tests(VectorFunction{std::vector{IntDataFunc{}, IntDataFunc{}}});
}

TEST_CASE("Combine data extraction functions into a vector", "[data]") {
	auto data_func = VectorFunction<IntDataFunc>{{{1}, {2}}};
	auto model = get_model();

	data_func.before_reset(model);
	advance_to_root_node(model);
	auto const data = data_func.extract(model, false);
	STATIC_REQUIRE(std::is_same_v<std::remove_const_t<decltype(data)>, std::vector<int>>);
	REQUIRE(data[0] == 2);
	REQUIRE(data[1] == 3);
}
