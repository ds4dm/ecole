#include <memory>
#include <stdexcept>

#include <catch2/catch.hpp>

#include "ecole/environment/branching.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/observation/node-bipartite.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/whensolved.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("BranchEnv") {
	auto env = environment::
		Branching<observation::NodeBipartite, reward::IsDone, termination::WhenSolved>(
			observation::NodeBipartite{}, reward::IsDone{}, termination::WhenSolved{});

	SECTION("reset, reset, and delete") {
		env.reset(problem_file);
		env.reset(problem_file);
	}

	SECTION("reset, step, and delete") {
		env.reset(problem_file);
		env.step(0);
	}

	SECTION("run full trajectory") {
		auto run_trajectory = [](auto& env, std::string const& filename) {
			auto obs_done = env.reset(filename);
			auto done = std::get<bool>(obs_done);
			auto count = 0;
			while (!done) {
				auto obs_rew_done_info = env.step(0);
				done = std::get<bool>(obs_rew_done_info);
				++count;
			}
			REQUIRE(count > 0);
		};
		run_trajectory(env, problem_file);

		SECTION("Run another trajectory") { run_trajectory(env, problem_file); }
	}

	SECTION("manage errors") {
		auto guard = ScipNoErrorGuard{};
		env.reset(problem_file);
		REQUIRE_THROWS_AS(env.step(-1), scip::Exception);
	}
}
