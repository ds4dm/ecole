#pragma once

#include <map>
#include <string>

#include "ecole/dynamics/parts.hpp"
#include "ecole/export.hpp"
#include "ecole/none.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::dynamics {

/**
 * A Dictionnary of parameter names to parameter values.
 */
using ParamDict = std::map<std::string, scip::Param>;

class ECOLE_EXPORT ConfiguringDynamics : public DefaultSetDynamicsRandomState {
public:
	using Action = ParamDict;
	using ActionSet = NoneType;

	using DefaultSetDynamicsRandomState::set_dynamics_random_state;

	ECOLE_EXPORT auto reset_dynamics(scip::Model& model) const -> std::tuple<bool, ActionSet>;

	ECOLE_EXPORT auto step_dynamics(scip::Model& model, Action const& param_dict) const -> std::tuple<bool, ActionSet>;
};

}  // namespace ecole::dynamics
