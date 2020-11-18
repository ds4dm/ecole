#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/environment/configuring.hpp"
#include "ecole/information/nothing.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/constant.hpp"
#include "ecole/traits.hpp"

using namespace ecole;

#define STATIC_REQUIRE_SAME(A, B) STATIC_REQUIRE(std::is_same_v<A, B>)

TEST_CASE("Detect if reward function", "[trait]") {
	SECTION("Positive tests") { STATIC_REQUIRE(trait::is_reward_function_v<reward::Constant>); }

	SECTION("Negative tests") {
		STATIC_REQUIRE_FALSE(trait::is_reward_function_v<ecole::NoneType>);
		STATIC_REQUIRE_FALSE(trait::is_reward_function_v<observation::Nothing>);
		STATIC_REQUIRE_FALSE(trait::is_reward_function_v<environment::Configuring<>>);
	}
}

TEST_CASE("Detect if observation function", "[trait]") {
	SECTION("Positive tests") { STATIC_REQUIRE(trait::is_observation_function_v<observation::Nothing>); }

	SECTION("Negative tests") {
		STATIC_REQUIRE_FALSE(trait::is_observation_function_v<ecole::NoneType>);
		STATIC_REQUIRE_FALSE(trait::is_observation_function_v<environment::Configuring<>>);
	}
}

TEST_CASE("Detect if information function", "[trait]") {
	SECTION("Positive tests") { STATIC_REQUIRE(trait::is_information_function_v<information::Nothing>); }

	SECTION("Negative tests") {
		STATIC_REQUIRE_FALSE(trait::is_information_function_v<ecole::NoneType>);
		STATIC_REQUIRE_FALSE(trait::is_information_function_v<observation::Nothing>);
		STATIC_REQUIRE_FALSE(trait::is_information_function_v<environment::Configuring<>>);
	}
}

TEST_CASE("Detect if environment", "[trait]") {
	SECTION("Positive tests") { STATIC_REQUIRE(trait::is_environment_v<environment::Configuring<>>); }

	SECTION("Negative tests") {
		STATIC_REQUIRE_FALSE(trait::is_environment_v<dynamics::ConfiguringDynamics>);
		STATIC_REQUIRE_FALSE(trait::is_environment_v<observation::Nothing>);
	}
}

TEST_CASE("Detect if dynamics", "[trait]") {
	SECTION("Positive tests") { STATIC_REQUIRE(trait::is_dynamics_v<dynamics::ConfiguringDynamics>); }

	SECTION("Negative tests") {
		STATIC_REQUIRE_FALSE(trait::is_dynamics_v<environment::Configuring<>>);
		STATIC_REQUIRE_FALSE(trait::is_dynamics_v<observation::Nothing>);
	}
}

TEST_CASE("Detect data type", "[trait]") {
	STATIC_REQUIRE_SAME(trait::data_of_t<observation::Nothing>, ecole::NoneType);
}

TEST_CASE("Detect observation type", "[trait]") {
	STATIC_REQUIRE_SAME(trait::observation_of_t<observation::Nothing>, ecole::NoneType);
	STATIC_REQUIRE_SAME(trait::observation_of_t<environment::Configuring<>>, ecole::NoneType);
}

TEST_CASE("Detect information type", "[trait]") {
	STATIC_REQUIRE_SAME(trait::information_of_t<information::Nothing>, ecole::NoneType);
	STATIC_REQUIRE_SAME(trait::information_of_t<environment::Configuring<>>, ecole::NoneType);
}

TEST_CASE("Detect action type", "[trait]") {
	STATIC_REQUIRE_SAME(trait::action_of_t<environment::Configuring<>>, dynamics::ParamDict);
	STATIC_REQUIRE_SAME(trait::action_of_t<dynamics::ConfiguringDynamics>, dynamics::ParamDict);
}

TEST_CASE("Detect action set type", "[trait]") {
	STATIC_REQUIRE_SAME(trait::action_set_of_t<environment::Configuring<>>, ecole::NoneType);
	STATIC_REQUIRE_SAME(trait::action_set_of_t<dynamics::ConfiguringDynamics>, ecole::NoneType);
}
