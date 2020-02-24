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
using ParamDict = std::map<std::string, scip::ParamType>;

template <typename... EnvTypes>
class Configuring : public DefaultEnvironment<ParamDict, DefaultState, EnvTypes...> {
public:
	using Base = DefaultEnvironment<ParamDict, DefaultState, EnvTypes...>;
	using typename Base::Action;
	using typename Base::Observation;

	using Base::Base;

	bool reset_state(DefaultState& initial_state) override;
	bool step_state(DefaultState& state, ParamDict const& action) override;
};

/***********************************
 *  Implementation of Configuring  *
 ***********************************/

template <typename... EnvTypes>
bool Configuring<EnvTypes...>::reset_state(DefaultState&) {
	return false;
}

template <typename... EnvTypes>
bool Configuring<EnvTypes...>::step_state(
	DefaultState& state,
	ParamDict const& param_dict  //
) {
	for (auto const& name_value : param_dict)
		nonstd::visit(
			[&](auto&& val) { state.model.set_param(name_value.first, val); },
			name_value.second);
	state.model.solve();
	return true;
}

}  // namespace environment
}  // namespace ecole
