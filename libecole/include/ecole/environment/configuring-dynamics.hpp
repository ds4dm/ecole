#pragma once

#include <map>
#include <string>

#include "ecole/environment/dynamics.hpp"
#include "ecole/environment/state.hpp"
#include "ecole/none.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace environment {

/**
 * A Dictionnary of parameter names to parameter values.
 */
using ParamDict = std::map<std::string, scip::Param>;

class ConfiguringDynamics :
	public EnvironmentDynamics<ParamDict, NoneType, environment::State> {
public:
	std::tuple<bool, NoneType> reset_dynamics(State& initial_state) override;
	std::tuple<bool, NoneType>
	step_dynamics(State& state, ParamDict const& action) override;
};

}  // namespace environment
}  // namespace ecole
