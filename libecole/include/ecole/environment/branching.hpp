#pragma once

#include "ecole/environment/branching-dynamics.hpp"
#include "ecole/environment/environment-composer.hpp"
#include "ecole/observation/nodebipartite.hpp"
#include "ecole/reward/isdone.hpp"

namespace ecole {
namespace environment {

template <
	typename ObservationFunction = observation::NodeBipartite,
	typename RewardFunction = reward::IsDone>
using Branching = EnvironmentComposer<BranchingDynamics, ObservationFunction, RewardFunction>;

}  // namespace environment
}  // namespace ecole
