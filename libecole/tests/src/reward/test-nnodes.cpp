#include <memory>
#include <string>
#include <tuple>

#include <catch2/catch.hpp>

#include "ecole/environment/branching.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/nnodes.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("NNodes unit tests", "[unit][reward]") {
	reward::unit_tests(reward::NNodes{});
}

TEST_CASE(
	"NNodes returns the difference in the total number of processed nodes between two states",
	"[reward]") {

	auto reward_func = reward::NNodes{};
	auto model = get_model();  // a non-trivial instance is loaded

	SECTION("NNodes is zero before presolving") {
		reward_func.reset(model);
		REQUIRE(reward_func.obtain_reward(model) == 0);
	}

	SECTION("NNodes is one after root node processing") {
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		REQUIRE(reward_func.obtain_reward(model) == 1);
	}

	SECTION("NNodes is zero if the model state has not changed") {
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		REQUIRE(reward_func.obtain_reward(model) >= 0);
		REQUIRE(reward_func.obtain_reward(model) == 0);
	}

	SECTION("Reset NNodes counter") {
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		auto reward = reward_func.obtain_reward(model);
		model = get_model();
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		REQUIRE(reward_func.obtain_reward(model) == reward);
	}
}

TEST_CASE("NNodes rewards are consistent when used in a Branching environment", "[reward]") {

	int max_nnodes = 20;
	auto env = environment::Branching<observation::Nothing, reward::NNodes>{
		{},
		{},
		{
			{"presolving/maxrounds", 0},  // just to save time here
			{"limits/totalnodes", max_nnodes},
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
		// Assert that the cumulated reward (total number of nodes) is not higher than the maximum
		// number of nodes authorized in the environment
		REQUIRE(cum_reward <= max_nnodes);
	}
}
