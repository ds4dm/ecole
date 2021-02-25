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
	advance_to_root_node(model);
	auto const optional_obs = obs_func.extract(model, false);

	SECTION("Observation is not empty on non terminal state") { REQUIRE(optional_obs.has_value()); }

	SECTION("Observation features are not empty") {
		auto const& obs = optional_obs.value();
		REQUIRE(obs.column_features.size() > 0);
		REQUIRE(obs.row_features.size() > 0);
		REQUIRE(obs.edge_features.nnz() > 0);
	}

	SECTION("Observation features have matching shape") {
		auto const& obs = optional_obs.value();
		REQUIRE(obs.row_features.shape()[0] == obs.edge_features.shape[0]);
		REQUIRE(obs.column_features.shape()[0] == obs.edge_features.shape[1]);
		REQUIRE(obs.edge_features.indices.shape()[0] == 2);
		REQUIRE(obs.edge_features.indices.shape()[1] == obs.edge_features.nnz());
	}

	SECTION("Columns features are not all nan") {
		auto const& col_feat = optional_obs.value().column_features;
		for (std::size_t i = 0; i < col_feat.shape()[1]; ++i) {
			REQUIRE_FALSE(xt::all(xt::isnan(xt::col(col_feat, static_cast<std::ptrdiff_t>(i)))));
		}
	}

	SECTION("Row features are not all nan") {
		auto const& row_feat = optional_obs.value().row_features;
		for (std::size_t i = 0; i < row_feat.shape()[1]; ++i) {
			REQUIRE_FALSE(xt::all(xt::isnan(xt::row(row_feat, static_cast<std::ptrdiff_t>(i)))));
		}
	}
}
