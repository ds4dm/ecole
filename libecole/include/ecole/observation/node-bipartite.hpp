#pragma once

#include <array>
#include <cstddef>
#include <limits>
#include <memory>

#include <xtensor/xview.hpp>

#include "ecole/container.hpp"
#include "ecole/environment/state.hpp"
#include "ecole/observation/abstract.hpp"

namespace ecole {
namespace observation {

template <typename ContainerSelector = container::xtensor> class NodeBipartiteObs {
public:
	using dtype = double;
	using tensor_t = container::tensor_t<ContainerSelector, dtype, 2>;

	tensor_t var_feat;
};

template <typename ContainerSelector = container::xtensor>
class NodeBipartite :
	public ObservationFunction<
		NodeBipartiteObs<ContainerSelector>,
		environment::DefaultState> {
public:
	using Observation = NodeBipartiteObs<ContainerSelector>;
	using State = environment::DefaultState;
	using Base = ObservationFunction<Observation, State>;

	std::unique_ptr<Base> clone() const override;

	Observation get(State const& state) override;
};

/****************************************
 *  Implementation of NodeBipartite  *
 ****************************************/

template <typename T> static constexpr T nan() {
	return std::numeric_limits<T>::quiet_NaN();
}

template <typename CS> auto NodeBipartite<CS>::clone() const -> std::unique_ptr<Base> {
	return std::make_unique<NodeBipartite>(*this);
}

template <typename CS> auto NodeBipartite<CS>::get(State const& state) -> Observation {
	auto const extract_feat = [](auto const var, auto out_iter) {
		*out_iter = var.ub_local().value_or(nan<double>());
	};

	auto var_feat = decltype(Observation::var_feat)::from_shape(
		std::array<std::size_t, 2>{state.model.variables().size, 1});
	std::size_t idx = 0;
	for (auto var : state.model.variables()) {
		extract_feat(var, xt::view(var_feat, idx++).begin());
	}
	return {std::move(var_feat)};
}

}  // namespace observation
}  // namespace ecole
