#include <array>
#include <cstddef>
#include <limits>

#include <xtensor/xview.hpp>

#include "ecole/observation/node-bipartite.hpp"

namespace ecole {
namespace observation {

template <typename T> static constexpr T nan() {
	return std::numeric_limits<T>::quiet_NaN();
}

auto NodeBipartite::clone() const -> std::unique_ptr<Base> {
	return std::make_unique<NodeBipartite>(*this);
}

auto NodeBipartite::get(environment::State const& state) -> NodeBipartiteObs {
	auto const extract_feat = [](auto const var, auto out_iter) {
		*out_iter = var.ub_local().value_or(nan<double>());
	};

	auto var_feat = decltype(NodeBipartiteObs::col_feat)::from_shape(
		std::array<std::size_t, 2>{state.model.variables().size, 1});
	std::size_t idx = 0;
	for (auto var : state.model.variables()) {
		extract_feat(var, xt::view(var_feat, idx++).begin());
	}
	return {std::move(var_feat)};
}

}  // namespace observation
}  // namespace ecole
