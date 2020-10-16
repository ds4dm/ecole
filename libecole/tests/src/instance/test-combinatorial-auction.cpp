#include <cstddef>

#include <catch2/catch.hpp>

#include "ecole/instance/combinatorial-auction.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("CombinatorialAuctionGenerator unit tests", "[unit][instance]") {
	// Keep problem size reasonable for tests
	std::size_t constexpr n_items = 50;
	std::size_t constexpr n_bids = 50;
	instance::unit_tests(instance::CombinatorialAuctionGenerator{{n_items, n_bids}});
}
