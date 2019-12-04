#include <algorithm>

#include "ecole/observation.hpp"

namespace ecole {
namespace obs {

BasicObs BasicObsSpace::get(scip::Model const& model) const {
	auto const var_view = model.variables();
	auto obs = BasicObs{};
	obs.ubs.resize(var_view.size);
	std::transform(
		var_view.begin(), var_view.end(), obs.ubs.begin(), [](auto var) { return var.ub(); });
	return obs;
}

auto BasicObsSpace::clone() const -> std::unique_ptr<ObservationSpace> {
	return std::make_unique<BasicObsSpace>(*this);
}

}  // namespace obs
}  // namespace ecole
