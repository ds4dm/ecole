#pragma once

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"
#include "ecole/utility/sparse_matrix.hpp"

namespace ecole {
namespace observation {

class NodeBipartiteObs {
public:
	using value_type = double;

	xt::xtensor<value_type, 2> column_features;
	xt::xtensor<value_type, 2> row_features;
	utility::coo_matrix<value_type> edge_features;
};

class NodeBipartite : public ObservationFunction<nonstd::optional<NodeBipartiteObs>> {
public:
	using Observation = nonstd::optional<NodeBipartiteObs>;
	using Base = ObservationFunction<Observation>;

	nonstd::optional<NodeBipartiteObs> obtain_observation(scip::Model const& model) override;
};

}  // namespace observation
}  // namespace ecole
