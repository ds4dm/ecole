#include <catch2/catch.hpp>

#include "ecole/instance/combinatorial-auction.hpp"

using namespace ecole;

TEST_CASE("CombinatorialAuctionGenerator unit tests", "[instance]") {
	auto generator = instance::CombinatorialAuctionGenerator{};

	static auto constexpr n_instances = 10;
	for (auto i = 0; i < n_instances; ++i) {
		auto model = generator.next();
	}
}
