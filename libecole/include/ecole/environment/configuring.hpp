#pragma once

#include "ecole/dynamics/configuring.hpp"
#include "ecole/environment/environment.hpp"
#include "ecole/information/nothing.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/isdone.hpp"

namespace ecole::environment {

template <
	typename ObservationFunction = observation::Nothing,
	typename RewardFunction = reward::IsDone,
	typename InformationFunction = information::Nothing>
using Configuring =
	Environment<dynamics::ConfiguringDynamics, ObservationFunction, RewardFunction, InformationFunction>;

}  // namespace ecole::environment
