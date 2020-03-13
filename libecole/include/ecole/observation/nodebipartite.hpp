#pragma once

#include <memory>

#include <xtensor/xtensor.hpp>

#include "ecole/environment/state.hpp"
#include "ecole/observation/abstract.hpp"
#include "ecole/utility/sparse_matrix.hpp"

namespace ecole {
namespace observation {

class NodeBipartiteObs {
public:
	using value_type = double;

	xt::xtensor<value_type, 2> col_feat;
	xt::xtensor<value_type, 2> row_feat;
	utility::coo_matrix<value_type> matrix;
};

class NodeBipartite : public ObservationFunction<NodeBipartiteObs> {
public:
	using Base = ObservationFunction<Observation>;

	std::unique_ptr<Base> clone() const override;

	NodeBipartiteObs get(environment::State const& state) override;
};

}  // namespace observation
}  // namespace ecole
