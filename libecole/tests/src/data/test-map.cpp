#include <string>
#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/data/map.hpp"

#include "conftest.hpp"
#include "data/mock-function.hpp"
#include "data/unit-tests.hpp"

using namespace ecole::data;

TEST_CASE("Data MapFunction unit tests", "[unit][data]") {
	ecole::data::unit_tests(MapFunction<std::string, IntDataFunc>{{{"a", {}}, {"b", {}}}});
}

TEST_CASE("Combine data extraction functions into a map", "[data]") {
	auto data_func = MapFunction<std::string, IntDataFunc>{{{"a", {1}}, {"b", {2}}}};
	auto model = get_model();

	data_func.before_reset(model);
	advance_to_root_node(model);
	auto const data = data_func.extract(model, false);
	STATIC_REQUIRE(std::is_same_v<std::remove_const_t<decltype(data)>, std::map<std::string, int>>);
	REQUIRE(data.at("a") == 2);
	REQUIRE(data.at("b") == 3);
}
