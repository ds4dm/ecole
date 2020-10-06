#include <catch2/catch.hpp>

#include "ecole/instance/independent-set.hpp"

using namespace ecole;

TEST_CASE("IndependentSetGenerator unit test", "[instance]") {
	auto generator = instance::IndependentSetGenerator{};

	static auto constexpr n_instances = 10;
	for (auto i = 0; i < n_instances; ++i) {
		auto model = generator.next();
	}
}
