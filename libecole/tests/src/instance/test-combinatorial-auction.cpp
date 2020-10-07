#include <catch2/catch.hpp>

#include "ecole/instance/combinatorial-auction.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("CombinatorialAuctionGenerator unit tests", "[unit][instance]") {
	instance::unit_tests(instance::CombinatorialAuctionGenerator{});
}
