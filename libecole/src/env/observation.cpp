#include "ecole/env/observation.hpp"

namespace ecole {
namespace env {

BasicObs BasicObsSpace::get(scip::Model const& model) {
	auto const var_view = model.variables();
	auto obs = BasicObs{};
	obs.ubs.resize(var_view.size);
	std::transform(
		var_view.begin(), var_view.end(), obs.ubs.begin(), [](auto var) { return var.ub(); });
	return obs;
}

} // namespace env
} // namespace ecole
