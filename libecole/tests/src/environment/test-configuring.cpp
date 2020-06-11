#include <memory>
#include <string>

#include <catch2/catch.hpp>

#include "ecole/environment/configuring.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/constant.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Configuring environment", "[env]") {
	auto env = environment::Configuring<observation::Nothing, reward::IsDone, termination::Constant>(
		{}, {}, {});
	auto policy = [](auto const&) { return decltype(env)::Action{{"branching/scorefunc", 's'}}; };

	SECTION("reset, reset, and delete") {
		env.reset(problem_file);
		env.reset(problem_file);
	}

	SECTION("reset, step, and delete") {
		decltype(env)::ActionSet action_set;
		std::tie(std::ignore, action_set, std::ignore, std::ignore) = env.reset(problem_file);
		env.step(policy(action_set));
	}

	for (auto i = 0; i < 2; ++i) {
		auto obs_as_reward_done = env.reset(problem_file);
		auto obs = std::get<0>(std::move(obs_as_reward_done));
		auto done = std::get<3>(std::move(obs_as_reward_done));

		// Assert that initial state is not terminal (episode length = 1)
		REQUIRE(!done);

		auto obs_rew_done_info = env.step({
			{"branching/scorefunc", 's'},
			{"branching/scorefac", 0.1},
			{"branching/divingpscost", false},
			{"conflict/lpiterations", 0},
			// std::string has lower priority than bool for converting const char*
			{"heuristics/undercover/fixingalts", std::string("ln")},
		});
		obs = std::get<0>(obs_rew_done_info);
		done = std::get<3>(obs_rew_done_info);

		// Assert that the second state is terminal (episode length = 1)
		REQUIRE(done);
	}
}
