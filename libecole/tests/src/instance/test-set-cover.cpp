#include <catch2/catch.hpp>

#include "ecole/instance/set-cover.hpp"

using namespace ecole;

TEST_CASE("Dummy test", "[instance]") {
	auto generator = instance::SetCoverGenerator{};

	constexpr static auto n_instances = 10;
	for (auto i = 0; i < n_instances; ++i) {
		auto model = generator.next();
	}
}
