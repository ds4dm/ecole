#pragma once

#include <tuple>

#include <catch2/catch.hpp>

#include "ecole/dynamics/dynamics.hpp"
#include "ecole/traits.hpp"

#include "conftest.hpp"

namespace ecole::dynamics {

template <typename Dynamics, typename Func> void unit_tests(Dynamics&& dyn, Func const policy) {
	auto model = get_model();

	SECTION("Has default constructor") { Dynamics{}; }

	SECTION("Perfom seeding") {
		RandomEngine random_engine{std::random_device{}()};
		RandomEngine random_engine_copy = random_engine;
		dyn.set_dynamics_random_state(model, random_engine);
		REQUIRE(random_engine != random_engine_copy);
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
		std::tie(done, action_set) = dyn.step_dynamics(model, policy(action_set));
	}

	SECTION("Run full trajectory") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		while (!done) {
			std::tie(done, action_set) = dyn.step_dynamics(model, policy(action_set));
		}

		SECTION("Run another trajectory") {
			model = get_model();
			std::tie(done, action_set) = dyn.reset_dynamics(model);
			while (!done) {
				std::tie(done, action_set) = dyn.step_dynamics(model, policy(action_set));
			}
		}
	}
}

}  // namespace ecole::dynamics
