#include <memory>
#include <string>
#include <tuple>

#include <catch2/catch.hpp>

#include "ecole/environment/branching.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/nnodes.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Using the NNodes reward in a Branching environment") {
	auto env = environment::Branching<observation::Nothing, reward::NNodes>{
		{},
		{},
		{
			{"presolving/maxrounds", 0},  // just to save time here
			{"limits/totalnodes", 20},
		},
		true};

	decltype(env)::ActionSet action_set;
	reward::Reward reward;
	bool done;

	for (auto i = 0; i < 2; ++i) {
		std::tie(std::ignore, action_set, reward, done) = env.reset(problem_file);

		auto cum_reward = reward;
		int n_steps = 0;

		// Assert that the number of nodes is non-negative
		REQUIRE(reward >= 0);
		// Assert that the cumulated reward (total number of nodes) is greater than the number of
		// branching steps
		REQUIRE(cum_reward >= n_steps);

		while (!done) {
			std::tie(std::ignore, action_set, reward, done, std::ignore) =
				env.step(action_set.value()[0]);  // dumb action

			cum_reward += reward;
			n_steps += 1;

			// Assert that the number of nodes is non-negative
			REQUIRE(reward >= 0);
			// Assert that the cumulated reward (total number of nodes) is greater than the number of
			// branching steps
			REQUIRE(cum_reward >= n_steps);
		}
	}
}
