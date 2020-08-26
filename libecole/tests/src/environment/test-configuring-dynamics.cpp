#include <string>
#include <tuple>

#include <catch2/catch.hpp>

#include "ecole/environment/configuring-dynamics.hpp"

#include "conftest.hpp"
#include "environment/unit-tests.hpp"

using namespace ecole;

TEST_CASE("ConfiguringDynamics unit tests", "[unit][dyn]") {
	auto const policy = [](auto const & /*action_set*/) -> trait::action_of_t<environment::ConfiguringDynamics> {
		return {{"branching/scorefunc", 's'}};
	};
	environment::unit_tests(environment::ConfiguringDynamics{}, policy);
}

TEST_CASE("ConfiguringDynamics functional tests", "[dyn]") {
	environment::ConfiguringDynamics dyn{};
	auto model = get_model();

	SECTION("Episodes have length one") {
		auto [done, action_set] = dyn.reset_dynamics(model);
		REQUIRE_FALSE(done);
		std::tie(done, std::ignore) = dyn.step_dynamics(model, {});
		REQUIRE(done);
	}

	SECTION("Solve instance") {
		dyn.reset_dynamics(model);
		dyn.step_dynamics(model, {});
		REQUIRE(model.is_solved());
	}

	SECTION("Accept multiple parameters") {
		trait::action_of_t<environment::ConfiguringDynamics> const params = {
			{"branching/scorefunc", 's'},
			{"branching/scorefac", 0.1},
			{"branching/divingpscost", false},
			{"conflict/lpiterations", 0},
			// std::string has lower priority than bool for converting const char*
			{"heuristics/undercover/fixingalts", std::string("ln")},
		};
		dyn.reset_dynamics(model);
		dyn.step_dynamics(model, params);
		for (auto const& name_val : params) {
			REQUIRE(name_val.second == model.get_param<decltype(name_val.second)>(name_val.first));
		}
	}
}
