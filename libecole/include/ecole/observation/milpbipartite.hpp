#pragma once

#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"
#include "ecole/utility/sparse-matrix.hpp"

namespace ecole::observation {

class MilpBipartiteObs {
public:
	using value_type = double;

	static inline std::size_t constexpr n_column_features = 5;
	enum struct ColumnFeatures : std::size_t {
		objective = 0,
		is_type_binary,            // One hot encoded
		is_type_integer,           // One hot encoded
		is_type_implicit_integer,  // One hot encoded
		is_type_continuous,        // One hot encoded
	};
	static inline std::size_t constexpr n_row_features = 2;
    enum struct RowFeatures : std::size_t {
		/** Static features */
		bias = 0,
		objective_cosine_similarity,
	};

	xt::xtensor<value_type, 2> column_features;
	xt::xtensor<value_type, 2> row_features;
	utility::coo_matrix<value_type> edge_features;
};

class MilpBipartite : public ObservationFunction<std::optional<MilpBipartiteObs>> {
public:
	NodeBipartite() {}

	std::optional<MilpBipartiteObs> extract(scip::Model& model, bool done) override;
};

}  // namespace ecole::observation
