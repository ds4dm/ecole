#pragma once

#include <map>
#include <string>

#include "ecole/environment/default.hpp"
#include "ecole/environment/state.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace environment {

/**
 * A Dictionnary of parameter names to parameter values.
 */
using ParamDict = std::map<std::string, scip::Param>;

class ConfiguringDynamics : public EnvironmentDynamics<ParamDict, environment::State> {
public:
	using Action = ParamDict;
	using State = environment::State;

	bool reset_dynamics(State& initial_state) override;
	bool step_dynamics(State& state, ParamDict const& action) override;
};

template <typename... EnvTypes>
using Configuring = EnvironmentComposer<ConfiguringDynamics, EnvTypes...>;

}  // namespace environment
}  // namespace ecole
