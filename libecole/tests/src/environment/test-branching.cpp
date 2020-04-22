#include <limits>
#include <memory>
#include <stdexcept>

#include <catch2/catch.hpp>

#include "ecole/environment/branching.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/observation/nodebipartite.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/whensolved.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("BranchEnv") {
	auto env = environment::
		Branching<observation::NodeBipartite, reward::IsDone, termination::WhenSolved>(
			{}, {}, {});

	SECTION("reset, reset, and delete") {
		env.reset(problem_file);
		env.reset(problem_file);
	}

	SECTION("reset, step, and delete") {
		env.reset(problem_file);
		env.step(0);
	}

	SECTION("run full trajectory") {
		auto run_trajectory = [&env](std::string const& filename) {
			auto obs_done = env.reset(filename);
			auto obs = std::get<0>(obs_done);
			auto done = std::get<1>(obs_done);
			auto count = 0;

			// Assert that the observation is none only on terminal states
			REQUIRE(obs.has_value() != done);

			while (!done) {
				auto obs_rew_done_info = env.step(0);
				obs = std::get<0>(obs_rew_done_info);
				done = std::get<2>(obs_rew_done_info);
				++count;

				// Assert that the observation is none only on terminal states
				REQUIRE(obs.has_value() != done);
			}

			REQUIRE(count > 0);
		};
		run_trajectory(problem_file);

		SECTION("Run another trajectory") { run_trajectory(problem_file); }
	}

	SECTION("manage errors") {
		auto guard = ScipNoErrorGuard{};
		env.reset(problem_file);
		auto const branch_var_too_large = std::numeric_limits<std::size_t>::max();
		REQUIRE_THROWS_AS(env.step(branch_var_too_large), scip::Exception);
	}
}
