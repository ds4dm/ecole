#pragma once

#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"
#include "ecole/utility/sparse-matrix.hpp"

namespace ecole::observation {

class MilpBipartiteObs {
public:
	using value_type = double;

	static inline std::size_t constexpr n_variable_features = 9;
	enum struct VariableFeatures : std::size_t {
		objective = 0,
		is_type_binary,            // One hot encoded
		is_type_integer,           // One hot encoded
		is_type_implicit_integer,  // One hot encoded
		is_type_continuous,        // One hot encoded
		has_lower_bound,
		has_upper_bound,
		lower_bound,
		upper_bound,
	};
	static inline std::size_t constexpr n_constraint_features = 1;
	enum struct ConstraintFeatures : std::size_t {
		bias = 0,
	};

	xt::xtensor<value_type, 2> variable_features;
	xt::xtensor<value_type, 2> constraint_features;
	utility::coo_matrix<value_type> edge_features;
};

class MilpBipartite : public ObservationFunction<std::optional<MilpBipartiteObs>> {
public:
	MilpBipartite(bool normalize_ = false) : normalize{normalize_} {}

	std::optional<MilpBipartiteObs> extract(scip::Model& model, bool done) override;

private:
	bool normalize = false;
};

}  // namespace ecole::observation
