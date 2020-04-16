#include "ecole/environment/configuring.hpp"

namespace ecole {
namespace environment {

bool ConfiguringDynamics::reset_state(State&) {
	return false;
}

bool ConfiguringDynamics::step_state(State& state, ParamDict const& param_dict) {
	for (auto const& name_value : param_dict)
		nonstd::visit(
			[&](auto&& val) { state.model.set_param(name_value.first, val); },
			name_value.second);
	state.model.solve();
	return true;
}

}  // namespace environment
}  // namespace ecole
