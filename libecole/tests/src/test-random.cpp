#include <catch2/catch.hpp>

#include "ecole/random.hpp"

using namespace ecole;

TEST_CASE("Same seed give same random generators", "[random]") {
	ecole::seed(0);
	auto rng_1 = ecole::spawn_random_generator();
	ecole::seed(0);
	auto rng_2 = ecole::spawn_random_generator();
	REQUIRE(rng_1 == rng_2);
}

TEST_CASE("Different seed give different random generators", "[random]") {
	ecole::seed(0);
	auto rng_1 = ecole::spawn_random_generator();
	ecole::seed(1);
	auto rng_2 = ecole::spawn_random_generator();
	REQUIRE(rng_1 != rng_2);
}

TEST_CASE("Successive random generators are different", "[random]") {
	auto rng_1 = ecole::spawn_random_generator();
	auto rng_2 = ecole::spawn_random_generator();
	REQUIRE(rng_1 != rng_2);
}

TEST_CASE("Random generator serialization", "[random]") {
	auto const rng = RandomGenerator{42};  // NOLINT This is deterministic for the test
	auto const data = serialize(rng);
	auto const rng_copy = deserialize(data);
	REQUIRE(rng == rng_copy);
}
