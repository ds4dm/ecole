#pragma once

#include <memory>

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/environment/state.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

struct BranchingResults {
	xt::xtensor<int, 2> candidates;
	xt::xtensor<double, 2> candidate_scores;
	int num_candidates;
	int best_candidate;
};


class StrongBranchingScoresObs {
public:

	xt::xtensor<double, 2> candidate_scores;
	xt::xtensor<double, 2> candidates;
	int num_candidates;
	int best_candidate;
};

class StrongBranchingScores : public ObservationFunction<nonstd::optional<StrongBranchingScoresObs>> {
public:
	using Observation = nonstd::optional<StrongBranchingScoresObs>;
	using Base = ObservationFunction<Observation>;

	nonstd::optional<StrongBranchingScoresObs>
	obtain_observation(environment::State const& state) override;
};

}  // namespace observation
}  // namespace ecole
