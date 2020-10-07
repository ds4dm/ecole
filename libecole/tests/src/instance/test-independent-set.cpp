#include <catch2/catch.hpp>

#include "ecole/instance/independent-set.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("IndependentSetGenerator unit test", "[unit][instance]") {
	instance::unit_tests(instance::IndependentSetGenerator{});
}
