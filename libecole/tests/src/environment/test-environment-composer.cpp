#include <tuple>

#include <catch2/catch.hpp>

#include "ecole/environment/dynamics.hpp"
#include "ecole/environment/environment-composer.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/none.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/constant.hpp"
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
	std::size_t const max_counter = 10;
	std::size_t counter = 0;
	double last_action = 0.;

	std::tuple<bool, NoneType> reset_dynamics(scip::Model& /*model*/) override {
		counter = 0;
		return {counter >= max_counter, None};
	}

	std::tuple<bool, NoneType> step_dynamics(scip::Model& /*model*/, double const& action) override {
		++counter;
		last_action = action;
		return {counter >= max_counter, None};
	}
};

using TestEnv = environment::EnvironmentComposer<TestDynamics, observation::Nothing, reward::Constant>;

}  // namespace environment
}  // namespace ecole

/**********************
 *  Test Environment  *
 **********************/

using namespace ecole;

TEST_CASE("Environments accept SCIP parameters", "[env]") {
	auto constexpr name = "concurrent/paramsetprefix";
	auto const value = std::string("testname");
	environment::TestEnv env{{}, {}, {{name, value}}};

	env.reset(problem_file);
	REQUIRE(env.model().get_param<std::string>(name) == std::string(value));
}

TEST_CASE("Environments have MDP API", "[env]") {
	auto env = environment::TestEnv{};
	auto obs = trait::observation_of_t<decltype(env)>{};
	auto action_set = trait::action_set_of_t<decltype(env)>{};
	auto reward = reward::Reward{0};
	auto done = false;
	constexpr double some_action = 3.0;

	SECTION("Call reset, reset, and delete") {
		std::tie(obs, action_set, reward, done) = env.reset(problem_file);
		std::tie(obs, action_set, reward, done) = env.reset(problem_file);
	}

	SECTION("Call reset, step, and delete") {
		std::tie(obs, action_set, reward, done) = env.reset(problem_file);
		std::tie(obs, action_set, reward, done, std::ignore) = env.step(some_action);
		REQUIRE(env.dynamics().last_action == some_action);
	}

	SECTION("Run full episodes") {
		for (auto i = 0UL; i < 2; ++i) {
			std::tie(obs, action_set, reward, done) = env.reset(problem_file);
			REQUIRE(env.dynamics().counter == 0UL);
			while (!done) {
				std::tie(obs, action_set, reward, done, std::ignore) = env.step(some_action);
			}
			REQUIRE(env.dynamics().counter == env.dynamics().max_counter);
		}
	}

	SECTION("Cannot transition without reseting") { REQUIRE_THROWS_AS(env.step(some_action), environment::Exception); }

	SECTION("Cannot transition past termination") {
		std::tie(std::ignore, std::ignore, std::ignore, done) = env.reset(problem_file);
		while (!done) {
			std::tie(std::ignore, std::ignore, std::ignore, done, std::ignore) = env.step(some_action);
		}
		REQUIRE_THROWS_AS(env.step(some_action), environment::Exception);
	}
}

/***************************
 *  Test default Dynamics  *
 ***************************/

TEST_CASE("Default Dynamics seed the Model", "[dyn]") {
	environment::TestEnv env{};
	constexpr auto some_seed = 93;
	env.seed(some_seed);
	env.reset(problem_file);
	auto seed1 = env.model().get_param<scip::Seed>("randomization/randomseedshift");
	env.seed(some_seed);
	env.reset(problem_file);
	auto seed2 = env.model().get_param<scip::Seed>("randomization/randomseedshift");
	REQUIRE(seed1 == seed2);
}

TEST_CASE("Defaut Dynamics change seed every episode", "[dyn]") {
	environment::TestEnv env{};
	env.reset(problem_file);
	auto seed1 = env.model().get_param<scip::Seed>("randomization/randomseedshift");
	env.reset(problem_file);
	auto seed2 = env.model().get_param<scip::Seed>("randomization/randomseedshift");
	REQUIRE(seed1 != seed2);
}
