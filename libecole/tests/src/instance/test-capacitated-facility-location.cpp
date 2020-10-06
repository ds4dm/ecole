#include <catch2/catch.hpp>

#include "ecole/instance/capacitated-facility-location.hpp"

using namespace ecole;

TEST_CASE("CapaciteatedFacilityLocationGenerator unit test", "[instance]") {
	auto generator = instance::CapacitatedFacilityLocationGenerator{};

	static auto constexpr n_instances = 10;
	for (auto i = 0; i < n_instances; ++i) {
		auto model = generator.next();
	}
}
