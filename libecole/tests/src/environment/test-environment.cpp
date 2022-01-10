#include <tuple>
#include <vector>

#include <catch2/catch.hpp>

#include "ecole/environment/environment.hpp"
#include "ecole/exception.hpp"
#include "ecole/information/nothing.hpp"
#include "ecole/none.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/random.hpp"
#include "ecole/reward/constant.hpp"
#include "ecole/traits.hpp"

#include "conftest.hpp"

/****************************************
 *  Mocking some classes for unit test  *
 ****************************************/

namespace ecole {
namespace dynamics {

/**
 * Dummy dynamics that record calls for testing purposes.
 */
struct TestDynamics {
	using Action = double;

	enum class Calls { seed, reset, step };

	static std::size_t constexpr max_call_lenght = 10;
	std::vector<Calls> calls;
	Action last_action = 0.;

	auto set_dynamics_random_state(scip::Model& /*model*/, RandomGenerator& /*rng*/) -> void {
		calls.push_back(Calls::seed);
	}

	auto reset_dynamics(scip::Model& /*model*/) -> std::tuple<bool, NoneType> {
		calls.push_back(Calls::reset);
		return {calls.size() >= max_call_lenght, None};
	}

	auto step_dynamics(scip::Model& /*model*/, double const& action) -> std::tuple<bool, NoneType> {
		calls.push_back(Calls::step);
		last_action = action;
		return {calls.size() >= max_call_lenght, None};
	}
};

}  // namespace dynamics

namespace environment {

using TestEnv = Environment<dynamics::TestDynamics, observation::Nothing, reward::Constant, information::Nothing>;

}  // namespace environment
}  // namespace ecole

/**********************
 *  Test Environment  *
 **********************/

using namespace ecole;

TEST_CASE("Environments accept SCIP parameters", "[env]") {
	auto constexpr name = "concurrent/paramsetprefix";
	auto const value = std::string("testname");
	auto env = environment::TestEnv{{}, {}, {}, {{name, value}}};

	env.reset(problem_file);
	REQUIRE(env.model().get_param<std::string>(name) == std::string(value));
}

TEST_CASE("Environments have MDP API", "[env]") {
	auto env = environment::TestEnv{};
	constexpr double some_action = 3.0;
	using Calls = dynamics::TestDynamics::Calls;

	SECTION("Call reset, reset, and delete") {
		auto [obs, action_set, reward, done, info] = env.reset(problem_file);
		std::tie(obs, action_set, reward, done, info) = env.reset(problem_file);
		REQUIRE(env.dynamics().calls == std::vector{Calls::seed, Calls::reset, Calls::seed, Calls::reset});
	}

	SECTION("Call reset, step, and delete") {
		auto [obs, action_set, reward, done, info] = env.reset(problem_file);
		std::tie(obs, action_set, reward, done, info) = env.step(some_action);
		REQUIRE(env.dynamics().calls == std::vector{Calls::seed, Calls::reset, Calls::step});
		REQUIRE(env.dynamics().last_action == some_action);
	}

	SECTION("Run full episodes") {
		for (auto i = 0UL; i < 2; ++i) {
			auto [obs, action_set, reward, done, info] = env.reset(problem_file);
			REQUIRE(env.dynamics().calls.back() == Calls::reset);
			while (!done) {
				std::tie(obs, action_set, reward, done, info) = env.step(some_action);
			}
		}
	}

	SECTION("Cannot transition without reseting") { REQUIRE_THROWS_AS(env.step(some_action), MarkovError); }

	SECTION("Cannot transition past termination") {
		auto [obs, action_set, reward, done, info] = env.reset(problem_file);
		while (!done) {
			std::tie(std::ignore, std::ignore, std::ignore, done, std::ignore) = env.step(some_action);
		}
		REQUIRE_THROWS_AS(env.step(some_action), MarkovError);
	}
}
