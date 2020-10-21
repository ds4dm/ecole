#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/utility/vector.hpp"

using namespace ecole;

TEMPLATE_TEST_CASE("Arange return indices from 0 to n", "[utility]", int, double) {  // NOLINT
	auto constexpr test_size = 10;
	auto vec = utility::arange<TestType>(test_size);

	STATIC_REQUIRE(std::is_same_v<typename decltype(vec)::value_type, TestType>);
	REQUIRE(vec.size() == test_size);
	REQUIRE(vec[0] == TestType{0});
	REQUIRE(vec[test_size - 1] == TestType{test_size - 1});
}
