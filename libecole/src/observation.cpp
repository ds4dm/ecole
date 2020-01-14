#include "xtensor/xview.hpp"

#include "ecole/observation.hpp"

namespace ecole {
namespace obs {

auto BasicObsSpace::clone() const -> std::unique_ptr<ObservationSpace> {
	return std::make_unique<BasicObsSpace>(*this);
}

auto BasicObsSpace::get(scip::Model const& model) -> obs_t {
	auto const extract_feat = [](auto const var, auto out_iter) { *out_iter = var.ub(); };

	decltype(BasicObs::var_feat) var_feat({model.variables().size, 1});
	std::size_t idx = 0;
	for (auto var : model.variables()) {
		extract_feat(var, xt::view(var_feat, idx++).begin());
	}
	return {std::move(var_feat)};
}

}  // namespace obs
}  // namespace ecole
