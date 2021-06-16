#pragma once

#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"

namespace ecole::observation {

struct Hutter2011Obs {
	static inline std::size_t constexpr n_features = 33;

	enum struct Features : std::size_t {
		/* Problem size features */
		nb_variables = 0,
		nb_constraints,
		nb_nonzero_coefs,
		/* Variable-constraint graph features */
		variable_node_degree_mean,
		variable_node_degree_max,
		variable_node_degree_min,
		variable_node_degree_std,
		constraint_node_degree_mean,
		constraint_node_degree_max,
		constraint_node_degree_min,
		constraint_node_degree_std,
		/* Variable graph (VG) features */
		node_degree_mean,
		node_degree_max,
		node_degree_min,
		node_degree_std,
		node_degree_25q,
		node_degree_75q,
		// Not computed because too expensive
		// clustering_coef_mean,
		// clustering_coef_std,
		edge_density,
		/* LP features */
		lp_slack_mean,
		lp_slack_max,
		lp_slack_l2,
		lp_objective_value,
		/* Objective function features */
		objective_coef_m_std,
		objective_coef_n_std,
		objective_coef_sqrtn_std,
		/* Linear constraint matrix features */
		constraint_coef_mean,
		constraint_coef_std,
		constraint_var_coef_mean,
		constraint_var_coef_std,
		/* Variable type features */
		discrete_vars_support_size_mean,
		discrete_vars_support_size_std,
		ratio_unbounded_discrete_vars,
		ratio_continuous_vars,
		/* General Problem type features */
		// Not computed due to SCIP not supporting MIQP
		// problem_type,
		// nb_quadratic_constraints,
		// nb_quadratic_nonzero_coefs,
		// nb_quadratic_variables,
	};

	xt::xtensor<double, 1> features;
};

class Hutter2011 : public ObservationFunction<std::optional<Hutter2011Obs>> {
public:
	std::optional<Hutter2011Obs> extract(scip::Model& model, bool done) override;
};

}  // namespace ecole::observation
