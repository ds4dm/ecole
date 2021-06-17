#include <algorithm>
#include <cmath>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>
#include <scip/scip.h>
#include <xtensor/xadapt.hpp>
#include <xtensor/xindex_view.hpp>
#include <xtensor/xsort.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/hutter-2011.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/sparse-matrix.hpp"

#include "utility/graph.hpp"
#include "utility/math.hpp"

namespace ecole::observation {

namespace {

namespace views = ranges::views;

using Features = Hutter2011Obs::Features;
using value_type = decltype(Hutter2011Obs::features)::value_type;
using ConstraintMatrix = ecole::utility::coo_matrix<SCIP_Real>;
std::size_t constexpr cons_axis = 0;
std::size_t constexpr var_axis = 1;

/** Convert an enum to its underlying index. */
template <typename E> constexpr auto idx(E e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

/** [1-3] Problem size features. */
template <typename Tensor> void set_problem_size(Tensor&& out, ConstraintMatrix const& cons_matrix) {
	out[idx(Features::nb_variables)] = static_cast<value_type>(cons_matrix.shape[var_axis]);
	out[idx(Features::nb_constraints)] = static_cast<value_type>(cons_matrix.shape[cons_axis]);
	out[idx(Features::nb_nonzero_coefs)] = static_cast<value_type>(cons_matrix.nnz());
}

/** [4-11] Variable-constraint graph features. */
template <typename Tensor> void set_var_cons_degrees(Tensor&& out, ConstraintMatrix const& cons_matrix) {
	// A degree counter to be reused.
	auto degrees = std::vector<std::size_t>(std::max(cons_matrix.shape[var_axis], cons_matrix.shape[cons_axis]));

	{  // Compute variables degrees
		degrees.resize(cons_matrix.shape[var_axis]);
		for (auto var_idx : xt::row(cons_matrix.indices, var_axis)) {
			assert(var_idx < degrees.size());
			degrees[var_idx]++;
		}
		auto const var_stats = utility::compute_stats(degrees);
		out[idx(Features::variable_node_degree_mean)] = var_stats.mean;
		out[idx(Features::variable_node_degree_max)] = var_stats.max;
		out[idx(Features::variable_node_degree_min)] = var_stats.min;
		out[idx(Features::variable_node_degree_std)] = var_stats.stddev;
	}
	{  // Reset degree vector and compute constraint degrees
		degrees.resize(cons_matrix.shape[cons_axis]);
		std::fill(degrees.begin(), degrees.end(), 0);
		for (auto cons_idx : xt::row(cons_matrix.indices, cons_axis)) {
			assert(cons_idx < degrees.size());
			degrees[cons_idx]++;
		}
		auto const cons_stats = utility::compute_stats(degrees);
		out[idx(Features::constraint_node_degree_mean)] = cons_stats.mean;
		out[idx(Features::constraint_node_degree_max)] = cons_stats.max;
		out[idx(Features::constraint_node_degree_min)] = cons_stats.min;
		out[idx(Features::constraint_node_degree_std)] = cons_stats.stddev;
	}
}

/**
 * Compute the quantile of an xtensor expression.
 *
 * The quantiles are computed by linear interpolation of the two data points in which it falls.
 *
 * @param data The (unsorted) data from which to extract the quantiles.
 * @param percentages Quantiles to compute, between 0 and 1.
 */
template <typename E, typename QT, std::size_t QN> auto quantiles(E&& data, std::array<QT, QN> const& percentages) {
	static_assert(std::is_floating_point_v<QT>);
	assert(std::all_of(percentages.begin(), percentages.end(), [](auto p) { return (0 <= p) && (p <= 1); }));

	auto const data_size = static_cast<QT>(data.size());

	// IILF Fill an array with upper and lower index of each quantile
	auto const quantile_idx = [&]() {
		auto pos = std::array<std::size_t, 2 * QN>{};
		auto pos_iter = pos.begin();
		for (auto p : percentages) {
			auto const continuous_idx = p * data_size;
			*(pos_iter++) = static_cast<std::size_t>(std::floor(continuous_idx));
			*(pos_iter++) = static_cast<std::size_t>(std::ceil(continuous_idx));
		}
		return pos;
	}();

	// Partially sort data so that the element whose index are given by quantile_idx are in the correct postiion.
	auto const partially_sorted_data = xt::partition(std::forward<E>(data), quantile_idx);

	//
	auto quants = std::array<QT, QN>{};
	auto* quants_iter = quants.begin();
	for (auto p : percentages) {
		auto const continuous_idx = p * data_size;
		auto const down_idx = static_cast<std::size_t>(std::floor(continuous_idx));
		auto const down_val = static_cast<QT>(partially_sorted_data[down_idx]);
		auto const up_idx = static_cast<std::size_t>(std::floor(continuous_idx));
		auto const up_val = static_cast<QT>(partially_sorted_data[up_idx]);
		auto const frac = continuous_idx - std::floor(continuous_idx);
		*(quants_iter++) = (1 - frac) * down_val + frac * up_val;
	}
	return quants;
}

/** [12-17,20] Variable graph features. */
template <typename Tensor> void set_var_degrees(Tensor&& out, ConstraintMatrix const& matrix) {
	auto const n_var = matrix.shape[var_axis];
	auto const n_cons = matrix.shape[cons_axis];
	// Build variable graph.
	// TODO could be optimized if we know matrix.indices[cons_axis] is sorted (or sort it).
	auto graph = utility::Graph{n_var};
	for (std::size_t cons = 0; cons < n_cons; ++cons) {
		auto const vars =
			xt::eval(xt::filter(xt::row(matrix.indices, var_axis), xt::equal(xt::row(matrix.indices, cons_axis), cons)));
		auto const* const var_end = vars.end();
		for (auto const* var1_iter = vars.begin(); var1_iter < var_end; ++var1_iter) {
			for (auto const* var2_iter = var1_iter + 1; var2_iter < var_end; ++var2_iter) {
				if (!graph.are_connected(*var1_iter, *var2_iter)) {
					graph.add_edge({*var1_iter, *var2_iter});
				}
			}
		}
	}

	// Compute stats
	auto get_var_degree = [&graph](auto var) { return graph.neighbors(var).size(); };
	auto var_degrees = views::ints(0UL, n_var) | views::transform(get_var_degree) | ranges::to<std::vector>();

	auto const stats = utility::compute_stats(var_degrees);
	out[idx(Features::node_degree_mean)] = stats.mean;
	out[idx(Features::node_degree_max)] = stats.max;
	out[idx(Features::node_degree_min)] = stats.min;
	out[idx(Features::node_degree_std)] = stats.stddev;
	auto const quants = quantiles(xt::adapt(var_degrees), std::array<double, 2>{0.25, 0.75});
	out[idx(Features::node_degree_25q)] = quants[0];
	out[idx(Features::node_degree_75q)] = quants[1];
	auto const n_edges_complete_graph = static_cast<value_type>(n_var * (n_var - 1)) / 2.;
	out[idx(Features::edge_density)] = static_cast<value_type>(graph.n_edges()) / n_edges_complete_graph;
}

/** Solves the LP relaxation of a model by making a copy, and setting all its variables continuous. */
auto solve_lp_relaxation(scip::Model const& model) {
	auto relax_model = model.copy();
	auto* const relax_scip = relax_model.get_scip_ptr();
	auto const variables = relax_model.variables();

	// Change active variables to continuous
	for (auto* const var : variables) {
		SCIP_Bool infeasible = FALSE;
		scip::call(SCIPchgVarType, relax_scip, var, SCIP_VARTYPE_CONTINUOUS, &infeasible);
		assert(!infeasible);
	}

	// Change constraint variables to continuous
	for (auto* const constraint : relax_model.constraints()) {
		auto const cons_vars = scip::get_cons_vars(relax_scip, constraint);
		assert(cons_vars.has_value());
		for (auto* const var : cons_vars.value()) {
			SCIP_Bool infeasible = FALSE;
			scip::call(SCIPchgVarType, relax_scip, var, SCIP_VARTYPE_CONTINUOUS, &infeasible);
			assert(!infeasible);
		}
	}

	// Solve the LP
	scip::call(SCIPsolve, relax_scip);

	// Collect the solution
	// Note: technically this is the solution with respect to the copy model's active variables
	// Hopefully this should match 1-1 the original model's active variables?
	SCIP_SOL* optimal_sol = SCIPgetBestSol(relax_scip);
	SCIP_Real optimal_value = SCIPgetSolOrigObj(relax_scip, optimal_sol);
	auto optimal_sol_coefs = std::vector<SCIP_Real>(variables.size());
	scip::call(
		SCIPgetSolVals,
		relax_scip,
		optimal_sol,
		static_cast<int>(variables.size()),
		variables.data(),
		optimal_sol_coefs.data());

	return std::tuple{std::move(optimal_sol_coefs), optimal_value};
}

/** [21-24] LP based features. */
template <typename Tensor> void set_lp_based_features(Tensor&& out, scip::Model const& model) {
	auto const [lp_solution, lp_objective] = solve_lp_relaxation(model);

	// Compute the integer slack vector
	auto* const scip = const_cast<SCIP*>(model.get_scip_ptr());
	int const nb_integer_variables = SCIPgetNBinVars(scip) + SCIPgetNIntVars(scip);

	if (nb_integer_variables > 0) {
		// Compute the integer slack vector
		auto integer_slack = std::vector<value_type>(static_cast<std::size_t>(nb_integer_variables));
		std::size_t int_var_idx = 0;
		for (auto& variable : model.variables()) {
			if (SCIPvarIsIntegral(variable)) {
				auto lp_solution_coef = lp_solution[int_var_idx];
				integer_slack[int_var_idx] = std::abs(lp_solution_coef - std::round(lp_solution_coef));
				int_var_idx++;
			}
		}

		// Compute statistics of the integer slack vector
		auto const slack_stats = utility::compute_stats(integer_slack);
		auto const square = [](auto val) { return val * val; };
		auto const slack_l2_norm = ranges::accumulate(integer_slack | views::transform(square), 0.);

		out[idx(Features::lp_slack_mean)] = slack_stats.mean;
		out[idx(Features::lp_slack_max)] = slack_stats.max;
		out[idx(Features::lp_slack_l2)] = slack_l2_norm;
	} else {
		out[idx(Features::lp_slack_mean)] = 0;
		out[idx(Features::lp_slack_max)] = 0;
		out[idx(Features::lp_slack_l2)] = 0;
	}
	out[idx(Features::lp_objective_value)] = lp_objective;
}

/** [25-27] Objective function features. */
template <typename Tensor>
void set_obj_features(Tensor&& out, scip::Model const& model, ConstraintMatrix const& cons_matrix) {
	auto const variables = model.variables();
	auto coefficients_m = std::vector<value_type>{};
	auto coefficients_n = std::vector<value_type>{};
	auto coefficients_sqrtn = std::vector<value_type>{};

	auto nb_cons_of_vars = std::vector<value_type>(variables.size(), 0);
	for (std::size_t coef_idx = 0; coef_idx < cons_matrix.nnz(); ++coef_idx) {
		nb_cons_of_vars[cons_matrix.indices(var_axis, coef_idx)]++;
	}

	coefficients_m.resize(variables.size());
	coefficients_n.resize(variables.size());
	coefficients_sqrtn.resize(variables.size());
	auto const nb_constraints = static_cast<value_type>(cons_matrix.shape[cons_axis]);
	for (std::size_t var_idx = 0; var_idx < variables.size(); ++var_idx) {
		auto c = SCIPvarGetObj(variables[var_idx]);
		coefficients_m.push_back(c / nb_constraints);
		if (nb_cons_of_vars[var_idx] != 0) {
			coefficients_n.push_back(c / nb_cons_of_vars[var_idx]);
			coefficients_sqrtn.push_back(c / std::sqrt(nb_cons_of_vars[var_idx]));
		}
	}

	auto const coefficients_m_stats = utility::compute_stats(coefficients_m);
	auto const coefficients_n_stats = utility::compute_stats(coefficients_n);
	auto const coefficients_sqrtn_stats = utility::compute_stats(coefficients_sqrtn);

	out[idx(Features::objective_coef_m_std)] = coefficients_m_stats.stddev;
	out[idx(Features::objective_coef_n_std)] = coefficients_n_stats.stddev;
	out[idx(Features::objective_coef_sqrtn_std)] = coefficients_sqrtn_stats.stddev;
}

/** [28-31] Linear constraint matrix features. */
template <typename Tensor>
void set_cons_matrix_features(
	Tensor&& out,
	ConstraintMatrix const& cons_matrix,
	xt::xtensor<SCIP_Real, 1> const& cons_biases) {
	auto nb_constraints = cons_matrix.shape[cons_axis];
	auto normalized_coefs = std::vector<value_type>{};
	auto norm_abs_coefs_counts = std::vector<value_type>(nb_constraints, 0);
	auto norm_abs_coefs_means = std::vector<value_type>(nb_constraints, 0);
	auto norm_abs_coefs_ssms = std::vector<value_type>(nb_constraints, 0);

	for (std::size_t coef_idx = 0; coef_idx < cons_matrix.nnz(); ++coef_idx) {
		auto cons_idx = cons_matrix.indices(cons_axis, coef_idx);
		auto bias = cons_biases(cons_idx);

		if (bias != 0) {
			auto normalized_coef = cons_matrix.values(coef_idx) / bias;
			normalized_coefs.push_back(normalized_coef);

			normalized_coef = std::abs(normalized_coef);

			// Online update formula
			norm_abs_coefs_counts[cons_idx]++;
			auto count = norm_abs_coefs_counts[cons_idx];
			if (count == 1) {
				norm_abs_coefs_means[cons_idx] = normalized_coef;
			} else {
				// At least two elements
				auto const delta = normalized_coef - norm_abs_coefs_means[cons_idx];
				norm_abs_coefs_means[cons_idx] += delta / count;
				auto const delta2 = normalized_coef - norm_abs_coefs_means[cons_idx];
				norm_abs_coefs_ssms[cons_idx] += delta * delta2;
			}
		}
	}

	auto norm_abs_var_coefs = std::vector<value_type>(nb_constraints, 0);
	for (std::size_t cons_idx = 0; cons_idx < nb_constraints; ++cons_idx) {
		if (auto const ssm = norm_abs_coefs_ssms[cons_idx]; ssm != 0) {
			norm_abs_var_coefs[cons_idx] = norm_abs_coefs_means[cons_idx] * norm_abs_coefs_counts[cons_idx] / ssm;
		}
	}

	auto const normalized_coefs_stats = utility::compute_stats(normalized_coefs);
	auto const norm_abs_var_coefs_stats = utility::compute_stats(norm_abs_var_coefs);

	out[idx(Features::constraint_coef_mean)] = normalized_coefs_stats.mean;
	out[idx(Features::constraint_coef_std)] = normalized_coefs_stats.stddev;
	out[idx(Features::constraint_var_coef_mean)] = norm_abs_var_coefs_stats.mean;
	out[idx(Features::constraint_var_coef_std)] = norm_abs_var_coefs_stats.stddev;
}

/** [32-35] Variable type features. */
template <typename Tensor> void set_variable_type_features(Tensor&& out, scip::Model const& model) {
	auto* const scip = const_cast<SCIP*>(model.get_scip_ptr());
	double nb_unbounded_int_vars = 0.;

	auto support_sizes = std::vector<std::size_t>{};
	for (auto* const variable : model.variables()) {
		if (SCIPvarGetType(variable) == SCIP_VARTYPE_BINARY) {
			support_sizes.push_back(2);
		} else if (SCIPvarGetType(variable) == SCIP_VARTYPE_INTEGER) {
			auto ub = SCIPvarGetUbGlobal(variable);
			auto lb = SCIPvarGetLbGlobal(variable);
			if (SCIPisInfinity(scip, std::abs(ub)) || SCIPisInfinity(scip, std::abs(lb))) {
				nb_unbounded_int_vars++;
			} else {
				support_sizes.push_back(static_cast<std::size_t>(ub - lb));
			}
		}
	}

	auto const support_sizes_stats = utility::compute_stats(support_sizes);
	auto const nb_int_vars = static_cast<double>(SCIPgetNBinVars(scip) + SCIPgetNIntVars(scip));
	auto const nb_cont_vars = static_cast<double>(SCIPgetNContVars(scip));

	out[idx(Features::discrete_vars_support_size_mean)] = support_sizes_stats.mean;
	out[idx(Features::discrete_vars_support_size_std)] = support_sizes_stats.stddev;
	if (nb_int_vars > 0) {
		out[idx(Features::ratio_unbounded_discrete_vars)] = nb_unbounded_int_vars / nb_int_vars;
	} else {
		out[idx(Features::ratio_unbounded_discrete_vars)] = 1.0;
	}
	out[idx(Features::ratio_continuous_vars)] = nb_cont_vars / (nb_int_vars + nb_cont_vars);
}

auto extract_features(scip::Model& model) {
	auto observation = xt::xtensor<value_type, 1>::from_shape({Hutter2011Obs::n_features});
	auto const [cons_matrix, cons_biases] = scip::get_all_constraints(model.get_scip_ptr());

	set_problem_size(observation, cons_matrix);
	set_var_cons_degrees(observation, cons_matrix);
	set_var_degrees(observation, cons_matrix);
	set_lp_based_features(observation, model);
	set_obj_features(observation, model, cons_matrix);
	set_cons_matrix_features(observation, cons_matrix, cons_biases);
	set_variable_type_features(observation, model);

	return observation;
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

auto Hutter2011::extract(scip::Model& model, bool /* done */) -> std::optional<Hutter2011Obs> {
	if (model.stage() >= SCIP_STAGE_SOLVING) {
		return {};
	}
	return {{extract_features(model)}};
}

}  // namespace ecole::observation
