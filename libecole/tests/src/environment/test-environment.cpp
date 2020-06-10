#include <tuple>

#include <catch2/catch.hpp>

#include "ecole/environment/default.hpp"
#include "ecole/environment/dynamics.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/none.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/constant.hpp"
#include "ecole/termination/constant.hpp"
#include "ecole/traits.hpp"

#include "conftest.hpp"

/****************************************
 *  Mocking some classes for unit test  *
 ****************************************/

namespace ecole {
namespace environment {

/**
 * Dummy dynamics that record calls for testing purposes.
 */
struct TestDynamics : EnvironmentDynamics<double, NoneType> {
	std::size_t max_counter = 10;
	std::size_t counter = 0;
	double last_action = 0.;

	std::tuple<bool, NoneType> reset_dynamics(scip::Model&) override {
		counter = 0;
		return {counter >= max_counter, None};
	}

	std::tuple<bool, NoneType> step_dynamics(scip::Model&, double const& action) override {
		++counter;
		last_action = action;
		return {counter >= max_counter, None};
	}
};

using TestEnv = environment::
	EnvironmentComposer<TestDynamics, observation::Nothing, reward::Constant, termination::Constant>;

}  // namespace environment
}  // namespace ecole

/**********************
 *  Test Environment  *
 **********************/

using namespace ecole;

TEST_CASE("Environments accept SCIP parameters", "[env]") {
	auto constexpr name = "concurrent/paramsetprefix";
	auto const value = std::string("testname");
	environment::TestEnv env{{}, {}, {}, {{name, value}}};

	env.reset(problem_file);
	REQUIRE(env.model().get_param<std::string>(name) == std::string(value));
}

TEST_CASE("Environments have MDP API", "[env]") {
	auto env = environment::TestEnv{};
	auto obs = trait::observation_of_t<decltype(env)>{};
	auto action_set = trait::action_set_of_t<decltype(env)>{};
	auto reward = reward::Reward{0};
	auto done = false;

	SECTION("Call reset, reset, and delete") {
		std::tie(obs, action_set, done) = env.reset(problem_file);
		std::tie(obs, action_set, done) = env.reset(problem_file);
	}

	SECTION("Call reset, step, and delete") {
		std::tie(obs, action_set, done) = env.reset(problem_file);
		std::tie(obs, action_set, reward, done, std::ignore) = env.step(3.0);
		REQUIRE(env.dynamics().last_action == 3.0);
	}

	SECTION("Run full episodes") {
		for (auto i = 0ul; i < 2; ++i) {
			std::tie(obs, action_set, done) = env.reset(problem_file);
			REQUIRE(env.dynamics().counter == 0ul);
			while (!done) {
				std::tie(obs, action_set, reward, done, std::ignore) = env.step(3.0);
			}
			REQUIRE(env.dynamics().counter == env.dynamics().max_counter);
		}
	}

	SECTION("Cannot transition without reseting") {
		REQUIRE_THROWS_AS(env.step(3.), environment::Exception);
	}

	SECTION("Cannot transition past termination") {
		std::tie(std::ignore, std::ignore, done) = env.reset(problem_file);
		while (!done) {
			std::tie(std::ignore, std::ignore, std::ignore, done, std::ignore) = env.step(3.0);
		}
		REQUIRE_THROWS_AS(env.step(3.), environment::Exception);
	}
}

/***************************
 *  Test default Dynamics  *
 ***************************/

TEST_CASE("Default Dynamics seed the Model", "[dynamics]") {
	environment::TestEnv env{};
	env.seed(93);
	env.reset(problem_file);
	auto seed1 = env.model().get_param<scip::Seed>("randomization/randomseedshift");
	env.seed(93);
	env.reset(problem_file);
	auto seed2 = env.model().get_param<scip::Seed>("randomization/randomseedshift");
	REQUIRE(seed1 == seed2);
}

TEST_CASE("Defaut Dynamics change seed every episode", "[dynamics]") {
	environment::TestEnv env{};
	env.reset(problem_file);
	auto seed1 = env.model().get_param<scip::Seed>("randomization/randomseedshift");
	env.reset(problem_file);
	auto seed2 = env.model().get_param<scip::Seed>("randomization/randomseedshift");
	REQUIRE(seed1 != seed2);
}
