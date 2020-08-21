#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/environment/configuring.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/traits.hpp"

using namespace ecole;

#define STATIC_REQUIRE_SAME(A, B) STATIC_REQUIRE(std::is_same<A, B>::value)

TEST_CASE("Detect if observation function", "[trait]") {
	SECTION("Positive tests") { STATIC_REQUIRE(trait::is_observation_function<observation::Nothing>::value); }

	SECTION("Negative tests") {
		STATIC_REQUIRE_FALSE(trait::is_observation_function<ecole::NoneType>::value);
		STATIC_REQUIRE_FALSE(trait::is_observation_function<environment::Configuring<>>::value);
	}
}

TEST_CASE("Detect if environment", "[trait]") {
	SECTION("Positive tests") { STATIC_REQUIRE(trait::is_environment<environment::Configuring<>>::value); }

	SECTION("Negative tests") {
		STATIC_REQUIRE_FALSE(trait::is_environment<environment::ConfiguringDynamics>::value);
		STATIC_REQUIRE_FALSE(trait::is_environment<observation::Nothing>::value);
	}
}

TEST_CASE("Detect if dynamics", "[trait]") {
	SECTION("Positive tests") { STATIC_REQUIRE(trait::is_dynamics<environment::ConfiguringDynamics>::value); }

	SECTION("Negative tests") {
		STATIC_REQUIRE_FALSE(trait::is_dynamics<environment::Configuring<>>::value);
		STATIC_REQUIRE_FALSE(trait::is_dynamics<observation::Nothing>::value);
	}
}

TEST_CASE("Detect observation type", "[trait]") {
	STATIC_REQUIRE_SAME(trait::observation_of_t<observation::Nothing>, ecole::NoneType);
	STATIC_REQUIRE_SAME(trait::observation_of_t<environment::Configuring<>>, ecole::NoneType);
}

TEST_CASE("Detect action type", "[trait]") {
	STATIC_REQUIRE_SAME(trait::action_of_t<environment::Configuring<>>, environment::ParamDict);
	STATIC_REQUIRE_SAME(trait::action_of_t<environment::ConfiguringDynamics>, environment::ParamDict);
}

TEST_CASE("Detect action set type", "[trait]") {
	STATIC_REQUIRE_SAME(trait::action_set_of_t<environment::Configuring<>>, ecole::NoneType);
	STATIC_REQUIRE_SAME(trait::action_set_of_t<environment::ConfiguringDynamics>, ecole::NoneType);
}
