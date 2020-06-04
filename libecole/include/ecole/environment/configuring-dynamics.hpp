#pragma once

#include <map>
#include <string>

#include "ecole/environment/dynamics.hpp"
#include "ecole/none.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace environment {

/**
 * A Dictionnary of parameter names to parameter values.
 */
using ParamDict = std::map<std::string, scip::Param>;

class ConfiguringDynamics : public EnvironmentDynamics<ParamDict, NoneType> {
public:
	std::tuple<bool, NoneType> reset_dynamics(scip::Model& model) override;
	std::tuple<bool, NoneType>
	step_dynamics(scip::Model& model, ParamDict const& action) override;
};

}  // namespace environment
}  // namespace ecole
