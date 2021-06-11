#include <algorithm>
#include <optional>
#include <utility>
#include <vector>
#include <cmath>

#include <scip/scip.h>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/hutter-2011.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/sparse-matrix.hpp"

#include "utility/math.hpp"

#include <iostream>

namespace ecole::observation {

namespace {

using Features = Hutter2011Obs::Features;
using value_type = decltype(Hutter2011Obs::features)::value_type;
using ConstraintMatrix = ecole::utility::coo_matrix<SCIP_Real>;
std::size_t constexpr cons_axis = 0;
std::size_t constexpr var_axis = 1;

/** Convert an enum to its underlying index. */
template <typename E> constexpr auto idx(E e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

template <typename Tensor> void set_problem_size(Tensor&& out, ConstraintMatrix const& cons_matrix) {
	out[idx(Features::nb_variables)] = static_cast<value_type>(cons_matrix.shape[var_axis]);
	out[idx(Features::nb_constraints)] = static_cast<value_type>(cons_matrix.shape[cons_axis]);
	out[idx(Features::nb_nonzero_coefs)] = static_cast<value_type>(cons_matrix.nnz());
}

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

/*
 * Solves the LP relaxation of a model by making a copy, and setting all its variables continuous.
 */
auto solve_lp_relaxation(scip::Model const& model) {
	auto relax_model = model.copy();
	auto* const relax_scip = relax_model.get_scip_ptr();
	auto* const variables = SCIPgetVars(relax_scip);
	int nb_variables = SCIPgetNVars(relax_scip);
	SCIP_Bool infeasible;

	// Change active variables to continuous
	for (std::size_t var_idx = 0; var_idx < static_cast<std::size_t>(nb_variables); ++var_idx) {
		scip::call(SCIPchgVarType, relax_scip, variables[var_idx], SCIP_VARTYPE_CONTINUOUS, &infeasible);
	}

	// Change constraint variables to continuous
	int nb_cons_variables;
	SCIP_Bool success;
	for (auto* const constraint : relax_model.constraints()) {
		scip::call(SCIPgetConsNVars, relax_scip, constraint, &nb_cons_variables, &success);
		auto cons_variables = std::vector<SCIP_VAR*>(static_cast<std::size_t>(nb_cons_variables));
		scip::call(SCIPgetConsVars, relax_scip, constraint, cons_variables.data(), nb_cons_variables, &success);

		for (std::size_t var_idx = 0; var_idx < static_cast<std::size_t>(nb_cons_variables); ++var_idx) {
			scip::call(SCIPchgVarType, relax_scip, cons_variables[var_idx], SCIP_VARTYPE_CONTINUOUS, &infeasible);
		}
	}

	// Solve the LP
	scip::call(SCIPsolve, relax_scip);

	// Collect the solution
	// Note: technically this is the solution with respect to the copy model's active variables
	// Hopefully this should match 1-1 the original model's active variables?
	SCIP_SOL* optimal_sol = SCIPgetBestSol(relax_scip);
	SCIP_Real optimal_value = SCIPgetSolOrigObj(relax_scip, optimal_sol);
	auto optimal_sol_coefs = std::vector<SCIP_Real>(static_cast<std::size_t>(nb_variables));
	scip::call(SCIPgetSolVals, relax_scip, optimal_sol, nb_variables, variables, optimal_sol_coefs.data());

	return std::tuple{optimal_sol_coefs, optimal_value};
}

template <typename Tensor> void set_lp_based_features(Tensor&& out, scip::Model const& model) {
	auto [lp_solution, lp_objective] = solve_lp_relaxation(model);

	// Compute the integer slack vector
	auto variables = model.variables();
	auto* const scip = const_cast<SCIP*>(model.get_scip_ptr());
	int nb_integer_variables = SCIPgetNBinVars(scip) + SCIPgetNIntVars(scip);

	if (nb_integer_variables > 0) {
		// Compute the integer slack vector
		auto integer_slack = std::vector<SCIP_Real>(static_cast<std::size_t>(nb_integer_variables));
		std::size_t int_var_idx = 0;
		for (auto& variable : variables) {
			if (SCIPvarIsIntegral(variable)) {
				auto lp_solution_coef = lp_solution[int_var_idx];
				integer_slack[int_var_idx] = std::abs(lp_solution_coef - std::round(lp_solution_coef));
				int_var_idx++;
			}
		}

		// Compute statistics of the integer slack vector
		auto const slack_stats = utility::compute_stats(integer_slack);
		SCIP_Real slack_l2_norm = 0;
		for (auto const coefficient : integer_slack) {
			slack_l2_norm += utility::square(coefficient);
		}

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
    

template <typename Tensor> void set_obj_features(Tensor&& out, scip::Model const& model, 
                                                 ConstraintMatrix const& cons_matrix) {
    auto variables = model.variables();
    auto coefficients_m = std::vector<SCIP_Real>(variables.size());
    auto coefficients_n = std::vector<SCIP_Real>(variables.size());
    auto coefficients_sqrtn = std::vector<SCIP_Real>(variables.size());
    
    auto nb_cons_of_vars = std::vector<long unsigned int>(variables.size(), 0);
    for (std::size_t coef_idx=0; coef_idx <  cons_matrix.nnz(); ++coef_idx) {
        nb_cons_of_vars[cons_matrix.indices(var_axis, coef_idx)]++;
    }
    
	auto* const scip = const_cast<SCIP*>(model.get_scip_ptr());
    int nb_constraints = SCIPgetNConss(scip);
	for (std::size_t var_idx = 0; var_idx < variables.size(); ++var_idx) {
        auto c = SCIPvarGetObj(variables[var_idx]);
        coefficients_m[var_idx] = c / nb_constraints;
        coefficients_n[var_idx] = c / static_cast<double>(nb_cons_of_vars[var_idx]);
        coefficients_n[var_idx] = c / std::sqrt(nb_cons_of_vars[var_idx]);
    }
    
    auto const coefficients_m_stats = utility::compute_stats(coefficients_m);
    auto const coefficients_n_stats = utility::compute_stats(coefficients_n);
    auto const coefficients_sqrtn_stats = utility::compute_stats(coefficients_sqrtn);
    
    out[idx(Features::objective_coef_m_std)] = coefficients_m_stats.stddev;
    out[idx(Features::objective_coef_n_std)] = coefficients_n_stats.stddev;
    out[idx(Features::objective_coef_sqrtn_std)] = coefficients_sqrtn_stats.stddev;
}

auto extract_features(scip::Model& model) {
	auto observation = xt::xtensor<value_type, 1>::from_shape({Hutter2011Obs::n_features});
	auto [cons_matrix, cons_biases] = scip::get_all_constraints(model.get_scip_ptr());

	set_problem_size(observation, cons_matrix);
	set_var_cons_degrees(observation, const_matrix);
	set_lp_based_features(observation, model);
	set_obj_features(observation, model, cons_matrix);

	return observation;
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

// void Hutter2011::before_reset(scip::Model& /* model */) {
// 	static_features = decltype(static_features){};
// }

auto Hutter2011::extract(scip::Model& model, bool /* done */) -> std::optional<Hutter2011Obs> {
	if (model.get_stage() >= SCIP_STAGE_SOLVING) {
		return {};
	}
	return {{extract_features(model)}};
}

}  // namespace ecole::observation