#include <catch2/catch.hpp>

#include "ecole/dynamics/parts.hpp"
#include "ecole/random.hpp"
#include "ecole/scip/model.hpp"

using namespace ecole;

TEST_CASE("Test default dynamics seeding", "[dynamics]") {
	auto dyn = dynamics::DefaultSetDynamicsRandomState{};
	auto rng = RandomGenerator{};  // NOLINT This is deterministic for the test
	auto model = scip::Model::prob_basic();

	SECTION("Random generator is consumed") {
		auto const rng_copy = rng;
		dyn.set_dynamics_random_state(model, rng);
		REQUIRE(rng != rng_copy);
	}

	SECTION("Defaut Dynamics change seed every episode") {
		dyn.set_dynamics_random_state(model, rng);
		auto const seed1 = model.get_param<scip::Seed>("randomization/randomseedshift");
		dyn.set_dynamics_random_state(model, rng);
		auto const seed2 = model.get_param<scip::Seed>("randomization/randomseedshift");
		REQUIRE(seed1 != seed2);
	}
}
