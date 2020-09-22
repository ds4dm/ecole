#include "ecole/dynamics/configuring.hpp"

namespace ecole::dynamics {

std::tuple<bool, NoneType> ConfiguringDynamics::reset_dynamics(scip::Model& /* model */) {
	return {false, None};
}

std::tuple<bool, NoneType> ConfiguringDynamics::step_dynamics(scip::Model& model, ParamDict const& param_dict) {
	for (auto const& [name, value] : param_dict) {
		model.set_param(name, value);
	}
	model.solve();
	return {true, None};
}

}  // namespace ecole::dynamics
