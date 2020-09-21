#pragma once

#include "ecole/environment/branching-dynamics.hpp"
#include "ecole/environment/environment.hpp"
#include "ecole/observation/nodebipartite.hpp"
#include "ecole/reward/isdone.hpp"

namespace ecole::environment {

template <typename ObservationFunction = observation::NodeBipartite, typename RewardFunction = reward::IsDone>
using Branching = Environment<BranchingDynamics, ObservationFunction, RewardFunction>;

}  // namespace ecole::environment
