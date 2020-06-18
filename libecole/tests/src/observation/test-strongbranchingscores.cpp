#include <catch2/catch.hpp>
#include <xtensor/xindex_view.hpp>
#include <xtensor/xmath.hpp>

#include "ecole/observation/strongbranchingscores.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

using namespace ecole;

TEST_CASE("StrongBranchingScores unit tests", "[unit][obs]") {
	unit_tests(observation::StrongBranchingScores{false});
}

TEST_CASE("StrongBranchingScores with pseudo candidates unit tests", "[unit][obs]") {
	unit_tests(observation::StrongBranchingScores{true});
}

TEST_CASE("StrongBranchingScores return correct branchig scores", "[obs]") {
	auto obs_func = observation::StrongBranchingScores{};
	auto model = get_solving_model();
	obs_func.reset(model);
	auto const obs = obs_func.obtain_observation(model);

	REQUIRE(obs.has_value());
	auto const scores = obs.value();
	REQUIRE(scores.size() == model.lp_columns().size);
	auto const not_nan_scores = xt::filter(scores, !xt::isnan(scores));
	REQUIRE(not_nan_scores.size() > 0);
	REQUIRE(xt::all(not_nan_scores >= 0));
}
