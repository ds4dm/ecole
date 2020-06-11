#include <limits>
#include <memory>
#include <stdexcept>
#include <tuple>

#include <catch2/catch.hpp>

#include "ecole/environment/branching.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/observation/strongbranchingscores.hpp"
#include "ecole/reward/isdone.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("StrongBranching") {
	environment::Branching<observation::StrongBranchingScores, reward::IsDone> env{};

	SECTION("reset, reset, and delete") {
		env.reset(problem_file);
		env.reset(problem_file);
	}

	SECTION("reset, step, and delete") {
		decltype(env)::ActionSet action_set;
		std::tie(std::ignore, action_set, std::ignore, std::ignore) = env.reset(problem_file);
		env.step(action_set.value()[0]);
	}

	SECTION("run full trajectory") {
		auto run_trajectory = [&env](std::string const& filename) {
			decltype(env)::Observation obs;
			decltype(env)::ActionSet action_set;
			bool done = false;
			reward::Reward reward;

			std::tie(obs, action_set, reward, done) = env.reset(filename);
			auto count = 0;

			// Assert that the observation is none only on terminal states
			REQUIRE(obs.has_value() != done);

			while (!done) {
				auto const action = action_set.value()[0];
				std::tie(obs, action_set, reward, done, std::ignore) = env.step(action);
				++count;

				// Assert that the observation is none only on terminal states
				REQUIRE(obs.has_value() != done);
			}

			REQUIRE(count > 0);
		};
		run_trajectory(problem_file);

		SECTION("Run another trajectory") { run_trajectory(problem_file); }
	}
}
