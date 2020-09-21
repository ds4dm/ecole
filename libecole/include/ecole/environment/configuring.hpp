#pragma once

#include "ecole/environment/configuring-dynamics.hpp"
#include "ecole/environment/environment.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/isdone.hpp"

namespace ecole::environment {

template <typename ObservationFunction = observation::Nothing, typename RewardFunction = reward::IsDone>
using Configuring = Environment<ConfiguringDynamics, ObservationFunction, RewardFunction>;

}  // namespace ecole::environment
