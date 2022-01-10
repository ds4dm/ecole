#pragma once

#include <tuple>

#include <catch2/catch.hpp>

#include "ecole/random.hpp"
#include "ecole/traits.hpp"

#include "conftest.hpp"

namespace ecole::dynamics {

template <typename Dynamics, typename Func> void unit_tests(Dynamics&& dyn, Func policy) {
	auto model = get_model();

	SECTION("Has default constructor") { Dynamics{}; }

	SECTION("Perfom seeding") {
		auto rng = RandomGenerator{std::random_device{}()};
		auto rng_copy = rng;
		dyn.set_dynamics_random_state(model, rng);
		REQUIRE(rng != rng_copy);
	}

	SECTION("Reset, reset, and delete") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
		model = get_model();
		std::tie(done, action_set) = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
	}

	SECTION("Reset, step, and delete") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
		std::tie(done, action_set) = dyn.step_dynamics(model, policy(action_set, model));
	}

	SECTION("Run full trajectory") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		while (!done) {
			std::tie(done, action_set) = dyn.step_dynamics(model, policy(action_set, model));
		}

		SECTION("Run another trajectory") {
			model = get_model();
			std::tie(done, action_set) = dyn.reset_dynamics(model);
			while (!done) {
				std::tie(done, action_set) = dyn.step_dynamics(model, policy(action_set, model));
			}
		}
	}
}

}  // namespace ecole::dynamics
