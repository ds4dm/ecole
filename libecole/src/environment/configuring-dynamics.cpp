#include "ecole/environment/configuring-dynamics.hpp"

namespace ecole {
namespace environment {

std::tuple<bool, NoneType> ConfiguringDynamics::reset_dynamics(scip::Model& /* model */) {
	return {false, None};
}

std::tuple<bool, NoneType>
ConfiguringDynamics::step_dynamics(scip::Model& model, ParamDict const& param_dict) {
	for (auto const& name_value : param_dict)
		nonstd::visit([&](auto&& val) { model.set_param(name_value.first, val); }, name_value.second);
	model.solve();
	return {true, None};
}

}  // namespace environment
}  // namespace ecole
