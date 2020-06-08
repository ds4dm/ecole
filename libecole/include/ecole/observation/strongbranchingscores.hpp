#pragma once

#include <memory>

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/environment/state.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

class StrongBranchingScoresObs {
public:
	xt::xtensor<double, 1> strong_branching_scores;
};

class StrongBranchingScores : public ObservationFunction<nonstd::optional<StrongBranchingScoresObs>> {
public:
	bool pseudo_cands;

	nonstd::optional<StrongBranchingScoresObs>
	obtain_observation(environment::State& state) override;
	StrongBranchingScores();
	StrongBranchingScores(bool pseudo_candidates);
};

}  // namespace observation
}  // namespace ecole
