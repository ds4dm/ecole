#pragma once

#include "ecole/environment/branching-dynamics.hpp"
#include "ecole/environment/default.hpp"
#include "ecole/observation/nodebipartite.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/constant.hpp"

namespace ecole {
namespace environment {

template <
	typename ObservationFunction = observation::NodeBipartite,
	typename RewardFunction = reward::IsDone,
	typename TerminationFunction = termination::Constant>
using Branching =
	EnvironmentComposer<BranchingDynamics, ObservationFunction, RewardFunction, TerminationFunction>;

}  // namespace environment
}  // namespace ecole
