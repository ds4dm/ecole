#pragma once

#include <map>
#include <string>

#include "ecole/dynamics/dynamics.hpp"
#include "ecole/export.hpp"
#include "ecole/none.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::dynamics {

/**
 * A Dictionnary of parameter names to parameter values.
 */
using ParamDict = std::map<std::string, scip::Param>;

class ECOLE_EXPORT ConfiguringDynamics : public EnvironmentDynamics<ParamDict, NoneType> {
public:
	ECOLE_EXPORT auto reset_dynamics(scip::Model& model) -> std::tuple<bool, NoneType> override;
	ECOLE_EXPORT auto step_dynamics(scip::Model& model, ParamDict const& param_dict)
		-> std::tuple<bool, NoneType> override;
};

}  // namespace ecole::dynamics
