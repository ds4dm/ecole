#include <catch2/catch.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/khalil-2016.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

using namespace ecole;

TEST_CASE("Khalil2016 unit tests", "[unit][obs]") {
	observation::unit_tests(observation::Khalil2016{});
}

TEST_CASE("Khalil2016 return correct observation", "[obs]") {
	auto obs_func = observation::Khalil2016{};
	auto model = get_solving_model();
	obs_func.reset(model);
	auto const optional_obs = obs_func.obtain_observation(model);

	SECTION("Observation is not empty on non terminal state") { REQUIRE(optional_obs.has_value()); }

	SECTION("Observation features has correct shape") {
		auto const obs = optional_obs.value();
		REQUIRE(obs.shape(0) == model.pseudo_branch_cands().size());
		REQUIRE(obs.shape(1) == 72);
	}

	SECTION("No features are NaN") {
		auto const obs = optional_obs.value();
		REQUIRE_FALSE(xt::any(xt::isnan(obs)));
	}
}
