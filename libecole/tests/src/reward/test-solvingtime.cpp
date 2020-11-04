#include <catch2/catch.hpp>

#include "ecole/environment/branching.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/solvingtime.hpp"

#include "conftest.hpp"
#include "reward/unit-tests.hpp"

using namespace ecole;

TEST_CASE("SolvingTime unit tests", "[unit][reward]") {
	bool wall = GENERATE(true, false);
	reward::unit_tests(reward::SolvingTime{wall});
}

TEST_CASE("Solving time rewards are positive initially", "[reward]") {
	bool wall = GENERATE(true, false);
	auto reward_func = reward::SolvingTime{wall};
	auto model = get_model();  // a non-trivial instance is loaded

	SECTION("Solving time is nonnegative before presolving") {
		reward_func.reset(model);
		REQUIRE(reward_func.obtain_reward(model) >= 0);
	}

	SECTION("Solving time is stricly positive after root node processing") {
		reward_func.reset(model);
		model.solve_iter();  // presolve and stop at the root node before branching
		REQUIRE(reward_func.obtain_reward(model) > 0);
	}
}

TEST_CASE("Solving time rewards are always strictly positive when used in a Branching environment", "[reward]") {
	constexpr int max_nnodes = 20;
	bool wall = GENERATE(true, false);
	auto env = environment::Branching<observation::Nothing, reward::SolvingTime>{
		{},
		{reward::SolvingTime{wall}},
		{
			{"presolving/maxrounds", 0},  // just to save time here
			{"limits/totalnodes", max_nnodes},
		},
		true};

	for (auto i = 0; i < 2; ++i) {
		auto [obs, action_set, reward, done] = env.reset(problem_file);

		// Assert that the number of nodes is strictly positive
		REQUIRE(reward > 0);

		while (!done) {
			std::tie(std::ignore, action_set, reward, done, std::ignore) = env.step(action_set.value()[0]);  // dumb action

			// Assert that the increase in solving time is strictly positive
			REQUIRE(reward > 0);
		}
	}
}
