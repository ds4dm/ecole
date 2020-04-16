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

class ConfiguringDynamics {
public:
	using Action = ParamDict;
	using State = environment::State;

	bool reset_state(State& initial_state);
	bool step_state(State& state, ParamDict const& action);
	void del_state(State&){};  // FIXME issue #24
};

template <typename... EnvTypes>
using Configuring = DefaultEnvironment<ConfiguringDynamics, EnvTypes...>;

}  // namespace environment
}  // namespace ecole
