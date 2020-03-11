#include <memory>
#include <string>

#include <catch2/catch.hpp>

#include "ecole/environment/configuring.hpp"
#include "ecole/observation/nodebipartite.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/whensolved.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Model creation") {
	auto env = environment::
		Configuring<observation::NodeBipartite, reward::IsDone, termination::WhenSolved>(
			observation::NodeBipartite{}, reward::IsDone{}, termination::WhenSolved{});

	for (auto i = 0L; i < 2; ++i) {
		auto obs_done = env.reset(problem_file);
		auto done = std::get<bool>(obs_done);

		REQUIRE(!done);
		// Test that the observation is none only on terminal states
		REQUIRE(std::get<0>(obs_done).has_value() != done);

		auto obs_rew_done_info = env.step({
			{"branching/scorefunc", 's'},
			{"branching/scorefac", 0.1},
			{"branching/divingpscost", false},
			{"conflict/lpiterations", 0},
			// std::string has lower priority than bool for converting const char*
			{"heuristics/undercover/fixingalts", std::string("ln")},
		});
		done = std::get<bool>(obs_rew_done_info);

		REQUIRE(done);
		// Test that the observation is none only on terminal states
		REQUIRE(std::get<0>(obs_rew_done_info).has_value() != done);
	}
}
