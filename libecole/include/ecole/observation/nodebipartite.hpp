#pragma once

#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"
#include "ecole/utility/sparse-matrix.hpp"

namespace ecole::observation {

struct NodeBipartiteObs {
	using value_type = double;

	static inline std::size_t constexpr n_static_column_features = 5;
	static inline std::size_t constexpr n_dynamic_column_features = 14;
	static inline std::size_t constexpr n_column_features = n_static_column_features + n_dynamic_column_features;
	enum struct ColumnFeatures : std::size_t {
		/** Static features */
		objective = 0,
		is_type_binary,            // One hot encoded
		is_type_integer,           // One hot encoded
		is_type_implicit_integer,  // One hot encoded
		is_type_continuous,        // One hot encoded

		/** Dynamic features */
		has_lower_bound,
		has_upper_bound,
		normed_reduced_cost,
		solution_value,
		solution_frac,
		is_solution_at_lower_bound,
		is_solution_at_upper_bound,
		scaled_age,
		incumbent_value,
		average_incumbent_value,
		is_basis_lower,  // One hot encoded
		is_basis_basic,  // One hot encoded
		is_basis_upper,  // One hot encoded
		is_basis_zero,   // One hot encoded
	};

	static inline std::size_t constexpr n_static_row_features = 2;
	static inline std::size_t constexpr n_dynamic_row_features = 3;
	static inline std::size_t constexpr n_row_features = n_static_row_features + n_dynamic_row_features;
	enum struct RowFeatures : std::size_t {
		/** Static features */
		bias = 0,
		objective_cosine_similarity,

		/** Dynamic features */
		is_tight,
		dual_solution_value,
		scaled_age,
	};

	xt::xtensor<value_type, 2> column_features;
	xt::xtensor<value_type, 2> row_features;
	utility::coo_matrix<value_type> edge_features;
};

class NodeBipartite : public ObservationFunction<std::optional<NodeBipartiteObs>> {
public:
	NodeBipartite(bool cache = false) : use_cache{cache} {}

	void before_reset(scip::Model& model) override;

	std::optional<NodeBipartiteObs> extract(scip::Model& model, bool done) override;

private:
	NodeBipartiteObs the_cache;
	bool use_cache = false;
	bool cache_computed = false;
};

}  // namespace ecole::observation
