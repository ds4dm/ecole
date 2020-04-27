#include "ecole/environment/configuring.hpp"

namespace ecole {
namespace environment {

std::tuple<bool, NoneType> ConfiguringDynamics::reset_dynamics(State&) {
	return {false, None};
}

std::tuple<bool, NoneType>
ConfiguringDynamics::step_dynamics(State& state, ParamDict const& param_dict) {
	for (auto const& name_value : param_dict)
		nonstd::visit(
			[&](auto&& val) { state.model.set_param(name_value.first, val); },
			name_value.second);
	state.model.solve();
	return {true, None};
}

}  // namespace environment
}  // namespace ecole
