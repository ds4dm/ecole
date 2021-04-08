#include <cstddef>

#include <catch2/catch.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/milpbipartite.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

using namespace ecole;

TEST_CASE("MilpBipartite unit tests", "[unit][obs]") {
	auto const normalize = GENERATE(true, false);
	observation::unit_tests(observation::MilpBipartite{normalize});
}

TEST_CASE("MilpBipartite return correct observation", "[obs]") {
	auto normalize = GENERATE(true, false);
	auto obs_func = observation::MilpBipartite{normalize};
	auto model = get_model();
	obs_func.before_reset(model);
	auto const optional_obs = obs_func.extract(model, false);

	SECTION("Observation is not empty on non terminal state") { REQUIRE(optional_obs.has_value()); }

	SECTION("Observation features are not empty") {
		auto const& obs = optional_obs.value();
		REQUIRE(obs.variable_features.size() > 0);
		REQUIRE(obs.constraint_features.size() > 0);
		REQUIRE(obs.edge_features.nnz() > 0);
	}

	SECTION("Observation features have matching shape") {
		auto const& obs = optional_obs.value();
		REQUIRE(obs.constraint_features.shape()[0] == obs.edge_features.shape[0]);
		REQUIRE(obs.variable_features.shape()[0] == obs.edge_features.shape[1]);
		REQUIRE(obs.edge_features.indices.shape()[0] == 2);
		REQUIRE(obs.edge_features.indices.shape()[1] == obs.edge_features.nnz());
	}

	SECTION("Variable features are not all nan") {
		auto const& var_feat = optional_obs.value().variable_features;
		for (std::size_t i = 0; i < var_feat.shape()[1]; ++i) {
			REQUIRE_FALSE(xt::all(xt::isnan(xt::col(var_feat, static_cast<std::ptrdiff_t>(i)))));
		}
	}

	SECTION("Constraint features are not all nan") {
		auto const& cons_feat = optional_obs.value().constraint_features;
		for (std::size_t i = 0; i < cons_feat.shape()[1]; ++i) {
			REQUIRE_FALSE(xt::all(xt::isnan(xt::row(cons_feat, static_cast<std::ptrdiff_t>(i)))));
		}
	}
}
