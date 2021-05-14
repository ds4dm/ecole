#include <catch2/catch.hpp>

#include "ecole/utility/chrono.hpp"

using namespace ecole;

TEST_CASE("cpu_clock is monotonic", "[utility]") {
	auto const before = utility::cpu_clock::now();
	auto const after = utility::cpu_clock::now();
	REQUIRE(before <= after);
}
