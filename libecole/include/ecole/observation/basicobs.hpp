#pragma once

#include <array>
#include <cstddef>
#include <memory>

#include <xtensor/xview.hpp>

#include "ecole/container.hpp"
#include "ecole/observation/base.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace observation {

template <typename ContainerSelector = container::xtensor> struct BasicObs {
	using dtype = double;
	using tensor_t = container::tensor_t<ContainerSelector, dtype, 2>;

	tensor_t var_feat;
};

template <typename ContainerSelector = container::xtensor>
struct BasicObsFunction : public ObservationFunction<BasicObs<ContainerSelector>> {
	using obs_t = BasicObs<ContainerSelector>;
	using base_t = ObservationFunction<obs_t>;

	std::unique_ptr<base_t> clone() const override;

	obs_t get(scip::Model const& model) override;
};

/****************************************
 *  Implementation of BasicObsFunction  *
 ****************************************/

template <typename CS>
auto BasicObsFunction<CS>::clone() const -> std::unique_ptr<base_t> {
	return std::make_unique<BasicObsFunction>(*this);
}

template <typename CS> auto BasicObsFunction<CS>::get(scip::Model const& model) -> obs_t {
	auto const extract_feat = [](auto const var, auto out_iter) { *out_iter = var.ub(); };

	auto var_feat = decltype(obs_t::var_feat)::from_shape(
		std::array<std::size_t, 2>{model.variables().size, 1});
	std::size_t idx = 0;
	for (auto var : model.variables()) {
		extract_feat(var, xt::view(var_feat, idx++).begin());
	}
	return {std::move(var_feat)};
}

}  // namespace observation
}  // namespace ecole
