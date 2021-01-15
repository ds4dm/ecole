#include <catch2/catch.hpp>
#include <xtensor/xindex_view.hpp>
#include <xtensor/xmath.hpp>

#include "ecole/observation/strongbranchingscores.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

using namespace ecole;

TEST_CASE("StrongBranchingScores unit tests", "[unit][obs]") {
	bool pseudo_candidates = GENERATE(true, false);
	observation::unit_tests(observation::StrongBranchingScores{pseudo_candidates});
}

TEST_CASE("StrongBranchingScores return correct branchig scores", "[obs]") {
	bool pseudo_candidates = GENERATE(true, false);
	auto obs_func = observation::StrongBranchingScores{pseudo_candidates};
	auto model = get_model();
	obs_func.before_reset(model);
	advance_to_root_node(model);
	auto const obs = obs_func.extract(model, true);

	REQUIRE(obs.has_value());
	auto const& scores = obs.value();
	REQUIRE(scores.size() == model.lp_columns().size());
	auto const not_nan_scores = xt::filter(scores, !xt::isnan(scores));
	REQUIRE(not_nan_scores.size() > 0);
	REQUIRE(xt::all(not_nan_scores >= 0));
}
