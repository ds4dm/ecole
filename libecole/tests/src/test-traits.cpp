#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/environment/configuring.hpp"
#include "ecole/information/abstract.hpp"
#include "ecole/reward/abstract.hpp"
#include "ecole/traits.hpp"

using namespace ecole;

#define STATIC_REQUIRE_SAME(A, B) STATIC_REQUIRE(std::is_same_v<A, B>)

TEST_CASE("Test Data functions traits", "[trait]") {
	SECTION("Succeed with ref to Model") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
			auto extract(scip::Model&, bool) -> double;
		};
		STATIC_REQUIRE(trait::is_data_function_v<Func>);
	}

	SECTION("Succeed with const ref to Model") {
		struct Func {
			auto before_reset(scip::Model const&) -> void;
			auto extract(scip::Model const&, bool) -> double;
		};
		STATIC_REQUIRE(trait::is_data_function_v<Func>);
	}

	SECTION("Fail when missing before_reset") {
		struct Func {
			auto extract(scip::Model&, bool) -> double;
		};
		STATIC_REQUIRE_FALSE(trait::is_data_function_v<Func>);
	}

	SECTION("Fail when before_reset takes Model by value") {
		struct Func {
			auto before_reset(scip::Model) -> void;
			auto extract(scip::Model&, bool) -> double;
		};
		STATIC_REQUIRE_FALSE(trait::is_data_function_v<Func>);
	}

	SECTION("Fail when before_reset returns non void") {
		struct Func {
			auto before_reset(scip::Model&) -> double;
			auto extract(scip::Model&, bool) -> double;
		};
		STATIC_REQUIRE_FALSE(trait::is_data_function_v<Func>);
	}

	SECTION("Fail when missing extract") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
		};
		STATIC_REQUIRE_FALSE(trait::is_data_function_v<Func>);
	}

	SECTION("Fail when extract takes Model by value") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
			auto extract(scip::Model, bool) -> double;
		};
		STATIC_REQUIRE_FALSE(trait::is_data_function_v<Func>);
	}

	SECTION("Fail when missing parameter in extract") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
			auto extract(scip::Model&) -> double;
		};
		STATIC_REQUIRE_FALSE(trait::is_data_function_v<Func>);
	}

	SECTION("Fail when extra parameter in extract") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
			auto extract(scip::Model&, bool, int) -> double;
		};
		STATIC_REQUIRE_FALSE(trait::is_data_function_v<Func>);
	}

	SECTION("Fail when extract returns void") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
			auto extract(scip::Model&, bool) -> void;
		};
		STATIC_REQUIRE_FALSE(trait::is_data_function_v<Func>);
	}
}

TEST_CASE("Test Reward functions traits", "[trait]") {
	SECTION("Succeed with double data") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
			auto extract(scip::Model&, bool) -> reward::Reward;
		};
		STATIC_REQUIRE(trait::is_reward_function_v<Func>);
	}

	SECTION("Fail with non Reward data") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
			auto extract(scip::Model&, bool) -> int;
		};
		STATIC_REQUIRE_FALSE(trait::is_reward_function_v<Func>);
	}
}

TEST_CASE("Test Information functions traits", "[trait]") {
	SECTION("Succeed with information map") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
			auto extract(scip::Model&, bool) -> information::InformationMap<double>;
		};
		STATIC_REQUIRE(trait::is_information_function_v<Func>);
	}

	SECTION("Fail with non infomation map") {
		struct Func {
			auto before_reset(scip::Model&) -> void;
			auto extract(scip::Model&, bool) -> int;
		};
		STATIC_REQUIRE_FALSE(trait::is_information_function_v<Func>);
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
	STATIC_REQUIRE_SAME(trait::observation_of_t<environment::Configuring<>>, std::optional<ecole::NoneType>);
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
