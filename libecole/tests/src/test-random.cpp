#include <catch2/catch.hpp>

#include "ecole/random.hpp"

using namespace ecole;

TEST_CASE("Same seed give same random engines", "[random]") {
	ecole::seed(0);
	auto random_engine_1 = ecole::spawn_random_engine();
	ecole::seed(0);
	auto random_engine_2 = ecole::spawn_random_engine();
	REQUIRE(random_engine_1 == random_engine_2);
}

TEST_CASE("Different seed give different random engines", "[random]") {
	ecole::seed(0);
	auto random_engine_1 = ecole::spawn_random_engine();
	ecole::seed(1);
	auto random_engine_2 = ecole::spawn_random_engine();
	REQUIRE(random_engine_1 != random_engine_2);
}

TEST_CASE("Successive random engines are different", "[random]") {
	auto random_engine_1 = ecole::spawn_random_engine();
	auto random_engine_2 = ecole::spawn_random_engine();
	REQUIRE(random_engine_1 != random_engine_2);
}

TEST_CASE("Random engine serialization", "[random]") {
	auto const engine = RandomEngine{42};  // NOLINT This is deterministic for the test
	auto const data = serialize(engine);
	auto const engine_copy = deserialize(data);
	REQUIRE(engine == engine_copy);
}
