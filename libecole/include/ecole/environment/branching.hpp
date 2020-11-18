#pragma once

#include "ecole/dynamics/branching.hpp"
#include "ecole/environment/environment.hpp"
#include "ecole/information/nothing.hpp"
#include "ecole/observation/nodebipartite.hpp"
#include "ecole/reward/isdone.hpp"

namespace ecole::environment {

template <
	typename ObservationFunction = observation::NodeBipartite,
	typename RewardFunction = reward::IsDone,
	typename InformationFunction = information::Nothing>
using Branching = Environment<dynamics::BranchingDynamics, ObservationFunction, RewardFunction, InformationFunction>;

}  // namespace ecole::environment
