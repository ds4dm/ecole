#include <catch2/catch.hpp>

#include "ecole/data/dynamic.hpp"

#include "conftest.hpp"
#include "data/mock-function.hpp"
#include "data/unit-tests.hpp"

using namespace ecole::data;

TEST_CASE("Dynamic function unit tests", "[unit][data]") {
	unit_tests(DynamicFunction<int>{IntDataFunc{}});
}

TEST_CASE("Dynamic function is polymorphic", "[unit][data]") {
	using Data = double;
	static constexpr auto int_val = 33;
	auto data_func = DynamicFunction<Data>{IntDataFunc{int_val}};
	auto model = get_model();

	SECTION("Extract correct data") {
		data_func.before_reset(model);
		auto const data = data_func.extract(model, false);
		STATIC_REQUIRE(std::is_same_v<std::remove_const_t<decltype(data)>, Data>);
		REQUIRE(data == Data{int_val + 1});
	}

	SECTION("Extract correct data when muted to new data function") {
		static constexpr auto double_val = 42.;
		data_func = DynamicFunction<Data>{DoubleDataFunc{double_val}};
		data_func.before_reset(model);
		auto const data = data_func.extract(model, false);
		STATIC_REQUIRE(std::is_same_v<std::remove_const_t<decltype(data)>, Data>);
		REQUIRE(data == double_val + 1);
	}
}
