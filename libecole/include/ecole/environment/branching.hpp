#pragma once

#include "ecole/dynamics/branching.hpp"
#include "ecole/environment/environment.hpp"
#include "ecole/observation/nodebipartite.hpp"
#include "ecole/reward/isdone.hpp"

namespace ecole::environment {

template <typename ObservationFunction = observation::NodeBipartite, typename RewardFunction = reward::IsDone>
using Branching = Environment<dynamics::BranchingDynamics, ObservationFunction, RewardFunction>;

}  // namespace ecole::environment
