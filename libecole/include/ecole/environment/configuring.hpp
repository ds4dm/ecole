#pragma once

#include "ecole/dynamics/configuring.hpp"
#include "ecole/environment/environment.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/isdone.hpp"

namespace ecole::environment {

template <typename ObservationFunction = observation::Nothing, typename RewardFunction = reward::IsDone>
using Configuring = Environment<dynamics::ConfiguringDynamics, ObservationFunction, RewardFunction>;

}  // namespace ecole::environment
