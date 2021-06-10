#include <algorithm>
#include <optional>
#include <utility>

#include <range/v3/view/map.hpp>
#include <range/v3/view/transform.hpp>
#include <robin_hood.h>
#include <scip/scip.h>
#include <xtensor/xtensor.hpp>

#include "ecole/observation/hutter-2011.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/sparse-matrix.hpp"

#include "utility/math.hpp"

namespace ecole::observation {

namespace {

namespace views = ranges::views;

using Features = Hutter2011Obs::Features;
using value_type = decltype(Hutter2011Obs::features)::value_type;

/** Convert an enum to its underlying index. */
template <typename E> constexpr auto idx(E e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

template <typename Tensor> void set_problem_size(Tensor&& out, scip::Model const& model) {
	auto* const scip = const_cast<SCIP*>(model.get_scip_ptr());
	out[idx(Features::nb_variables)] = static_cast<value_type>(SCIPgetNVars(scip));
	out[idx(Features::nb_constraints)] = static_cast<value_type>(SCIPgetNConss(scip));
	// SCIPgetNNZs return 0 at in problem stage so we use the degree statistics in `set_variable_degrees` instead.
}

template <typename Tensor> void set_variable_degrees(Tensor&& out, scip::Model const& model) {
	// Fill a map variable -> degree with all variables and 0.
	auto const variables = model.variables();
	auto var_degrees = robin_hood::unordered_flat_map<SCIP_VAR const*, std::size_t>{};
	var_degrees.reserve(variables.size());
	std::for_each(variables.begin(), variables.end(), [&var_degrees](auto var) { var_degrees[var] = 0; });

	// Compute degrees by iterating over the constraints.
	for (auto* const cons : model.constraints()) {
		auto maybe_cons_vars = scip::get_cons_vars(model.get_scip_ptr(), cons);
		if (maybe_cons_vars.has_value()) {
			auto const cons_vars = std::move(maybe_cons_vars).value();
			std::for_each(cons_vars.begin(), cons_vars.end(), [&var_degrees](auto var) { var_degrees[var]++; });
		}
	}

	auto const stats = utility::compute_stats(var_degrees | views::values);
	out[idx(Features::variable_node_degree_mean)] = stats.mean;
	out[idx(Features::variable_node_degree_max)] = stats.max;
	out[idx(Features::variable_node_degree_min)] = stats.min;
	out[idx(Features::variable_node_degree_std)] = stats.stddev;
	// See `set_problem_size`.
	out[idx(Features::nb_nonzero_coefs)] = stats.count;
}

template <typename Tensor> void set_constraint_degrees(Tensor&& out, scip::Model const& model) {
	auto cons_degree = [&model](auto cons) {
		return static_cast<value_type>(scip::get_cons_n_vars(model.get_scip_ptr(), cons).value_or(0));
	};
	auto const constraints = model.constraints();
	auto const stats = utility::compute_stats(constraints | views::transform(cons_degree));
	out[idx(Features::constraint_node_degree_mean)] = stats.mean;
	out[idx(Features::constraint_node_degree_max)] = stats.max;
	out[idx(Features::constraint_node_degree_min)] = stats.min;
	out[idx(Features::constraint_node_degree_std)] = stats.stddev;
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

auto extract_features(scip::Model& model) {
	xt::xtensor<value_type, 1> observation({Hutter2011Obs::n_features});
	auto constraint_matrix = utility::coo_matrix{};
	set_problem_size(observation, model);
	set_variable_degrees(observation, model);
	set_constraint_degrees(observation, model);
	set_lp_based_features(observation, model);

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
