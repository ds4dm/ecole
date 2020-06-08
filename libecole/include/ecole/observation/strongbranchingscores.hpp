#pragma once

#include <memory>

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/environment/state.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

class StrongBranchingScores :
	public ObservationFunction<nonstd::optional<xt::xtensor<double, 1>>> {
public:
	bool pseudo_cands;

	nonstd::optional<xt::xtensor<double, 1>>
	obtain_observation(environment::State& state) override;
	StrongBranchingScores();
	StrongBranchingScores(bool pseudo_candidates);
};

}  // namespace observation
}  // namespace ecole
