#pragma once

#include "ecole/environment/configuring-dynamics.hpp"
#include "ecole/environment/environment-composer.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/isdone.hpp"

namespace ecole::environment {

template <typename ObservationFunction = observation::Nothing, typename RewardFunction = reward::IsDone>
using Configuring = EnvironmentComposer<ConfiguringDynamics, ObservationFunction, RewardFunction>;

}  // namespace ecole::environment
