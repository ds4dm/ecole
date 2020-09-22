#pragma once

#include <map>
#include <string>

#include "ecole/dynamics/dynamics.hpp"
#include "ecole/none.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::dynamics {

/**
 * A Dictionnary of parameter names to parameter values.
 */
using ParamDict = std::map<std::string, scip::Param>;

class ConfiguringDynamics : public EnvironmentDynamics<ParamDict, NoneType> {
public:
	std::tuple<bool, NoneType> reset_dynamics(scip::Model& model) override;
	std::tuple<bool, NoneType> step_dynamics(scip::Model& model, ParamDict const& param_dict) override;
};

}  // namespace ecole::dynamics
