#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <catch2/catch.hpp>

#include "ecole/data/parser.hpp"
#include "ecole/none.hpp"

#include "conftest.hpp"
#include "data/mock-function.hpp"
#include "data/unit-tests.hpp"

using namespace ecole::data;

auto make_function_aggregate() {
	return std::tuple{std::map<std::string, IntDataFunc>{{"0", {}}}, std::vector{1.0}, ecole::None};
}

TEST_CASE("Data function parser passes unit tests", "[unit][data]") {
	ecole::data::unit_tests(parse(make_function_aggregate()));
}

TEST_CASE("Recursively parse data functions", "[data]") {
	auto aggregate_func = parse(make_function_aggregate());
	auto model = get_model();

	aggregate_func.before_reset(model);
	advance_to_root_node(model);
	auto const aggregate_obs = aggregate_func.extract(model, false);

	using AggregateObs = std::remove_const_t<decltype(aggregate_obs)>;
	STATIC_REQUIRE(
		std::is_same_v<AggregateObs, std::tuple<std::map<std::string, int>, std::vector<double>, ecole::NoneType>>);
	REQUIRE(std::get<0>(aggregate_obs).at("0") == 1);
	REQUIRE(std::get<1>(aggregate_obs).at(0) == 1.0);
}
