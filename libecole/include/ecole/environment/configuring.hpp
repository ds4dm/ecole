#pragma once

#include "ecole/environment/configuring-dynamics.hpp"
#include "ecole/environment/default.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/constant.hpp"

namespace ecole {
namespace environment {

template <
	typename ObservationFunction = observation::Nothing,
	typename RewardFunction = reward::IsDone,
	typename TerminationFunction = termination::Constant>
using Configuring = EnvironmentComposer<
	ConfiguringDynamics,
	ObservationFunction,
	RewardFunction,
	TerminationFunction>;

}  // namespace environment
}  // namespace ecole
