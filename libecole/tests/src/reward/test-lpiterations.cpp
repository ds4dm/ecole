#include <memory>
#include <string>

#include <catch2/catch.hpp>

#include "ecole/environment/configuring.hpp"
#include "ecole/observation/none.hpp"
#include "ecole/reward/lpiterations.hpp"
#include "ecole/termination/whensolved.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Using the reward in a Configuring environment") {
	auto env =
		environment::Configuring<observation::None, reward::LPIterations, termination::WhenSolved>(
			{}, {}, {});

	for (auto i = 0; i < 2; ++i) {
        env.seed(i);
		env.reset(problem_file);

		auto obs_rew_done_info = env.step({});
        auto reward = std::get<1>(obs_rew_done_info);

        // Assert that the reward is non-positive
		REQUIRE(reward <= 0);
	}

	for (auto i = 0; i < 2; ++i) {
        env.seed(i);
		env.reset(problem_file);

		auto obs_rew_done_info = env.step({
            {"presolving/maxrounds", 0},
			{"lp/iterlim", 0},
            {"lp/rootiterlim", 0},
			{"limits/totalnodes", 1},
		});
        auto reward = std::get<1>(obs_rew_done_info);

        // Assert that the reward is zero, since no LP iteration was allowed
		REQUIRE(reward == 0);
	}
}
