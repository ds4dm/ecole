#include <catch2/catch.hpp>

#include "ecole/instance/capacitated-facility-location.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("CapaciteatedFacilityLocationGenerator unit test", "[unit][instance]") {
	instance::unit_tests(instance::CapacitatedFacilityLocationGenerator{});
}
