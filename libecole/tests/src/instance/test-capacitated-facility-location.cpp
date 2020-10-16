#include <cstddef>

#include <catch2/catch.hpp>

#include "ecole/instance/capacitated-facility-location.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("CapaciteatedFacilityLocationGenerator unit test", "[unit][instance]") {
	// Keep problem size reasonable for tests
	std::size_t constexpr n_customers = 60;
	std::size_t constexpr n_facilities = 50;
	instance::unit_tests(instance::CapacitatedFacilityLocationGenerator{{n_customers, n_facilities}});
}
