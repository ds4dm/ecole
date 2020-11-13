#pragma once

#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"
#include "ecole/utility/sparse-matrix.hpp"

namespace ecole::observation {

class NodeBipartiteObs {
public:
	using value_type = double;

	xt::xtensor<value_type, 2> column_features;
	xt::xtensor<value_type, 2> row_features;
	utility::coo_matrix<value_type> edge_features;
};

class NodeBipartite : public ObservationFunction<std::optional<NodeBipartiteObs>> {
public:
	std::optional<NodeBipartiteObs> obtain_observation(scip::Model& model, bool done) override;
};

}  // namespace ecole::observation
