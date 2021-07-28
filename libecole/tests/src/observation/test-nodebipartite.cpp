#include <cstddef>

#include <catch2/catch.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/nodebipartite.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

using namespace ecole;

TEST_CASE("NodeBipartite unit tests", "[unit][obs]") {
	observation::unit_tests(observation::NodeBipartite{});
}

TEST_CASE("NodeBipartite return correct observation", "[obs]") {
	auto cache = GENERATE(true, false);
	auto obs_func = observation::NodeBipartite{cache};
	auto model = get_model();
	if (cache) {
		model.disable_cuts();
	}
	obs_func.before_reset(model);
	advance_to_stage(model, SCIP_STAGE_SOLVING);
	auto const optional_obs = obs_func.extract(model, false);

	SECTION("Observation is not empty on non terminal state") { REQUIRE(optional_obs.has_value()); }

	SECTION("Observation features are not empty") {
		auto const& obs = optional_obs.value();
		REQUIRE(obs.variable_features.size() > 0);
		REQUIRE(obs.row_features.size() > 0);
		REQUIRE(obs.edge_features.nnz() > 0);
	}

	SECTION("Observation features have matching shape") {
		auto const& obs = optional_obs.value();
		REQUIRE(obs.row_features.shape()[0] == obs.edge_features.shape[0]);
		REQUIRE(obs.variable_features.shape()[0] == obs.edge_features.shape[1]);
		REQUIRE(obs.edge_features.indices.shape()[0] == 2);
		REQUIRE(obs.edge_features.indices.shape()[1] == obs.edge_features.nnz());
	}

	SECTION("Variable features are not all nan") {
		auto const& obs = optional_obs.value();
		REQUIRE_FALSE(xt::all(xt::isnan(obs.variable_features)));
	}

	SECTION("Row features are not all nan") {
		auto const& obs = optional_obs.value();
		REQUIRE_FALSE(xt::all(xt::isnan(obs.row_features)));
	}
}
