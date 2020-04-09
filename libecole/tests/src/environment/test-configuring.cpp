#include <memory>
#include <string>

#include <catch2/catch.hpp>

#include "ecole/environment/configuring.hpp"
#include "ecole/observation/none.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/whensolved.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Model creation") {
	auto env =
		environment::Configuring<observation::None, reward::IsDone, termination::WhenSolved>(
			{}, {}, {});

	for (auto i = 0; i < 2; ++i) {
		auto obs_done = env.reset(problem_file);
		auto obs = std::get<0>(obs_done);
		auto done = std::get<1>(obs_done);

		// Assert that initial state is not terminal (episode length = 1)
		REQUIRE(!done);
		// Assert that an initial observation is returned
		REQUIRE(obs.has_value());

		auto obs_rew_done_info = env.step({
			{"branching/scorefunc", 's'},
			{"branching/scorefac", 0.1},
			{"branching/divingpscost", false},
			{"conflict/lpiterations", 0},
			// std::string has lower priority than bool for converting const char*
			{"heuristics/undercover/fixingalts", std::string("ln")},
		});
		obs = std::get<0>(obs_rew_done_info);
		done = std::get<2>(obs_rew_done_info);

		// Assert that the second state is terminal (episode length = 1)
		REQUIRE(done);
		// Assert that no observation is returned on terminal states
		REQUIRE(!obs.has_value());
	}
}
