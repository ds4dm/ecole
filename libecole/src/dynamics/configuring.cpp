#include "ecole/dynamics/configuring.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::dynamics {

auto ConfiguringDynamics::reset_dynamics(scip::Model& /* model */) const -> std::tuple<bool, NoneType> {
	return {false, None};
}

auto ConfiguringDynamics::step_dynamics(scip::Model& model, ParamDict const& param_dict) const
	-> std::tuple<bool, NoneType> {
	for (auto const& [name, value] : param_dict) {
		model.set_param(name, value);
	}
	model.solve();
	return {true, None};
}

}  // namespace ecole::dynamics
