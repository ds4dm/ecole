#pragma once

#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"
#include "ecole/utility/sparse-matrix.hpp"

namespace ecole::observation {

class NodeBipartiteObs {
public:
	using value_type = double;

	static constexpr std::size_t n_column_features = 19;
	enum struct ColumnFeatures : std::size_t {
		has_lower_bound = 0,
		has_upper_bound,
		normed_reduced_cost,
		objective,
		solution_value,
		solution_frac,
		is_solution_at_lower_bound,
		is_solution_at_upper_bound,
		scaled_age,
		is_basis_lower,  // One hot encoded
		is_basis_basic,  // One hot encoded
		is_basis_upper,  // One hot encoded
		is_basis_zero,   // One hot encoded
		incumbent_value,
		average_incumbent_value,
		is_type_binary,            // One hot encoded
		is_type_integer,           // One hot encoded
		is_type_implicit_integer,  // One hot encoded
		is_type_continuous,        // One hot encoded
	};

	static constexpr std::size_t n_row_features = 5;
	enum struct RowFeatures : std::size_t {
		bias = 0,
		is_tight,
		scaled_age,
		objective_cosine_similarity,
		dual_solution_value,
	};

	xt::xtensor<value_type, 2> column_features;
	xt::xtensor<value_type, 2> row_features;
	utility::coo_matrix<value_type> edge_features;
};

class NodeBipartite : public ObservationFunction<std::optional<NodeBipartiteObs>> {
public:
	std::optional<NodeBipartiteObs> extract(scip::Model& model, bool done) override;
};

}  // namespace ecole::observation
