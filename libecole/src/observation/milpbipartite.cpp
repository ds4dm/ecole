#include <type_traits>

#include <cmath>
#include <cstddef>
#include <fmt/format.h>
#include <scip/scip.h>
#include <scip/struct_lp.h>
#include <xtensor/xadapt.hpp>
#include <xtensor/xnorm.hpp>
#include <xtensor/xview.hpp>

#include "ecole/exception.hpp"
#include "ecole/observation/milpbipartite.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::observation {

namespace {

/*********************
 *  Common helpers   *
 *********************/

using xmatrix = decltype(MilpBipartiteObs::variable_features);
using value_type = xmatrix::value_type;
using coo_xmatrix = utility::coo_matrix<value_type>;

using VariableFeatures = MilpBipartiteObs::VariableFeatures;
using ConstraintFeatures = MilpBipartiteObs::ConstraintFeatures;

/******************************************
 *  Variable extraction functions         *
 ******************************************/

/* Computes the L2 norm of the objective.
	 This is done by hand because SCIPgetObjNorm is not available for all stages (need >= SCIP_STAGE_TRANSFORMED) */
SCIP_Real obj_l2_norm(SCIP* const scip, scip::Model& model) noexcept {
	SCIP_Real norm = 0.;

	if (SCIPgetStage(scip) >= SCIP_STAGE_TRANSFORMED) {
		norm = SCIPgetObjNorm(scip);
	} else {
		// If too early, this must be done by hand
		for (auto* variable : model.variables()) {
			norm += std::pow(SCIPvarGetObj(variable), 2);
		}
		norm = std::sqrt(norm);
	}

	return norm > 0 ? norm : 1.;
}

/** Convert an enum to its underlying index. */
template <typename E> constexpr auto idx(E e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

template <typename Features>
void set_static_features_for_var(
	Features&& out,
	SCIP* const scip,
	SCIP_VAR* const var,
	std::optional<value_type> obj_norm = {}) {
	double const objsense = (SCIPgetObjsense(scip) == SCIP_OBJSENSE_MINIMIZE) ? 1. : -1.;

	out[idx(VariableFeatures::objective)] = objsense * SCIPvarGetObj(var);
	if (obj_norm.has_value()) {
		out[idx(VariableFeatures::objective)] /= obj_norm.value();
	}
	// One-hot enconding of variable type
	out[idx(VariableFeatures::is_type_binary)] = 0.;
	out[idx(VariableFeatures::is_type_integer)] = 0.;
	out[idx(VariableFeatures::is_type_implicit_integer)] = 0.;
	out[idx(VariableFeatures::is_type_continuous)] = 0.;
	switch (SCIPvarGetType(var)) {
	case SCIP_VARTYPE_BINARY:
		out[idx(VariableFeatures::is_type_binary)] = 1.;
		break;
	case SCIP_VARTYPE_INTEGER:
		out[idx(VariableFeatures::is_type_integer)] = 1.;
		break;
	case SCIP_VARTYPE_IMPLINT:
		out[idx(VariableFeatures::is_type_implicit_integer)] = 1.;
		break;
	case SCIP_VARTYPE_CONTINUOUS:
		out[idx(VariableFeatures::is_type_continuous)] = 1.;
		break;
	default:
		assert(false);  // All enum cases must be handled
	}

	auto const lower_bound = SCIPvarGetLbLocal(var);
	if (SCIPisInfinity(scip, std::abs(lower_bound))) {
		out[idx(VariableFeatures::has_lower_bound)] = 0.;
		out[idx(VariableFeatures::lower_bound)] = 0.;
	} else {
		out[idx(VariableFeatures::has_lower_bound)] = 1.;
		out[idx(VariableFeatures::lower_bound)] = lower_bound;
	}

	auto const upper_bound = SCIPvarGetUbLocal(var);
	if (SCIPisInfinity(scip, std::abs(upper_bound))) {
		out[idx(VariableFeatures::has_upper_bound)] = 0.;
		out[idx(VariableFeatures::upper_bound)] = 0.;
	} else {
		out[idx(VariableFeatures::has_upper_bound)] = 1.;
		out[idx(VariableFeatures::upper_bound)] = upper_bound;
	}
}

void set_features_for_all_vars(xmatrix& out, scip::Model& model, bool normalize) {
	auto* const scip = model.get_scip_ptr();

	// Contant reused in every iterations
	auto const obj_norm = normalize ? std::optional{obj_l2_norm(scip, model)} : std::nullopt;

	auto const variables = model.variables();
	auto const n_vars = variables.size();
	for (std::size_t var_idx = 0; var_idx < n_vars; ++var_idx) {
		auto features = xt::row(out, static_cast<std::ptrdiff_t>(var_idx));
		set_static_features_for_var(features, scip, variables[var_idx], obj_norm);
	}
}

/****************************************
 *  Constraint extraction functions     *
 ****************************************/

SCIP_Real cons_l2_norm(std::vector<SCIP_Real> const& constraint_coefs) {
	auto xt_constraint_coefs = xt::adapt(constraint_coefs, {constraint_coefs.size()});

	auto const norm = xt::norm_l2(xt_constraint_coefs)();
	return norm > 0. ? norm : 1.;
}

/**
 * Obtains the variables involved in a linear constraint and their coefficients in the constraint
 */
auto get_constraint_linear_coefs(SCIP* const scip, SCIP_CONS* const constraint) -> std::optional<
	std::tuple<std::vector<SCIP_VAR*>, std::vector<SCIP_Real>, std::optional<SCIP_Real>, std::optional<SCIP_Real>>> {
	SCIP_Bool success = false;
	int n_constraint_variables;
	int n_active_variables;
	SCIP_Real constant_offset = 0;
	int requiredsize = 0;

	// Find how many active variables and constraint variables there are (for allocation)
	scip::call(SCIPgetConsNVars, scip, constraint, &n_constraint_variables, &success);
	if (!success) {
		return std::nullopt;
	}
	n_active_variables = SCIPgetNVars(scip);

	// Allocate buffers large enough to hold future variables and coefficients
	auto const buffer_size = static_cast<std::size_t>(std::max(n_constraint_variables, n_active_variables));
	auto variables = std::vector<SCIP_VAR*>(buffer_size);
	auto coefficients = std::vector<SCIP_Real>(buffer_size);

	// Get the variables and their coefficients in the constraint
	scip::call(SCIPgetConsVars, scip, constraint, variables.data(), static_cast<int>(buffer_size), &success);
	if (!success) {
		return std::nullopt;
	}
	scip::call(SCIPgetConsVals, scip, constraint, coefficients.data(), static_cast<int>(buffer_size), &success);
	if (!success) {
		return std::nullopt;
	}

	// If we are in SCIP_STAGE_TRANSFORMED or later, the variables in the constraint might be inactive
	// Re-express the coefficients in terms of active variables
	if (SCIPgetStage(scip) >= SCIP_STAGE_TRANSFORMED) {
		scip::call(
			SCIPgetProbvarLinearSum,
			scip,
			variables.data(),
			coefficients.data(),
			&n_constraint_variables,
			static_cast<int>(buffer_size),
			&constant_offset,
			&requiredsize,
			true);
	}

	variables.resize(static_cast<std::size_t>(n_constraint_variables));
	coefficients.resize(static_cast<std::size_t>(n_constraint_variables));

	// Obtain the left hand side
	std::optional<SCIP_Real> lhs;
	SCIP_Real lhs_value = SCIPconsGetLhs(scip, constraint, &success);
	if (success && !SCIPisInfinity(scip, std::abs(lhs_value))) {
		lhs = lhs_value - constant_offset;
	}

	std::optional<SCIP_Real> rhs;
	SCIP_Real rhs_value = SCIPconsGetRhs(scip, constraint, &success);
	if (success && !SCIPisInfinity(scip, std::abs(rhs_value))) {
		rhs = rhs_value - constant_offset;
	}

	return std::optional{std::tuple{variables, coefficients, lhs, rhs}};
}

auto get_constraint_coefs(SCIP* const scip, SCIP_CONS* const constraint)
	-> std::tuple<std::vector<SCIP_VAR*>, std::vector<SCIP_Real>, std::optional<SCIP_Real>, std::optional<SCIP_Real>> {
	auto constraint_data = get_constraint_linear_coefs(scip, constraint);
	if (constraint_data.has_value()) {  // Constraint must be linear
		return constraint_data.value();
	}
	throw Exception(fmt::format(
		"Constraint {} cannot be expressed as a single linear constraint (type \"{}\"), MilpBipartite observation "
		"cannot be extracted.",
		SCIPconsGetPos(constraint),
		SCIPconshdlrGetName(SCIPconsGetHdlr(constraint))));
}

auto extract_constraints(scip::Model& model, bool normalize) -> std::tuple<utility::coo_matrix<value_type>, xmatrix> {
	auto* const scip = model.get_scip_ptr();
	auto const constraints = model.constraints();

	std::size_t n_rows = 0;
	auto n_cols = static_cast<std::size_t>(SCIPgetNVars(scip));

	std::vector<value_type> values;
	std::vector<std::size_t> column_indices;
	std::vector<std::size_t> row_indices;
	std::vector<value_type> biases;

	// For each constraint
	for (std::size_t cons_idx = 0; cons_idx < std::size(constraints); ++cons_idx) {
		auto* const constraint = constraints[cons_idx];
		auto [constraint_vars, constraint_coefs, lhs, rhs] = get_constraint_coefs(scip, constraint);
		SCIP_Real const constraint_norm = normalize ? cons_l2_norm(constraint_coefs) : 1.;

		// Inequality has a left hand side?
		if (lhs.has_value()) {
			for (std::size_t cons_var_idx = 0; cons_var_idx < std::size(constraint_vars); ++cons_var_idx) {
				value_type value = constraint_coefs[cons_var_idx];
				int var_idx = SCIPvarGetProbindex(constraint_vars[cons_var_idx]);

				values.push_back(-value);
				row_indices.push_back(n_rows);
				column_indices.push_back(static_cast<std::size_t>(var_idx));
			}
			if (normalize) {
				biases.push_back(-lhs.value() / constraint_norm);
			} else {
				biases.push_back(-lhs.value());
			}
			n_rows++;
		}
		// Inequality has a right hand side?
		if (rhs.has_value()) {
			for (std::size_t cons_var_idx = 0; cons_var_idx < std::size(constraint_vars); ++cons_var_idx) {
				value_type value = constraint_coefs[cons_var_idx];
				int var_idx = SCIPvarGetProbindex(constraint_vars[cons_var_idx]);

				values.push_back(value);
				row_indices.push_back(n_rows);
				column_indices.push_back(static_cast<std::size_t>(var_idx));
			}
			if (normalize) {
				biases.push_back(rhs.value() / constraint_norm);
			} else {
				biases.push_back(rhs.value());
			}
			n_rows++;
		}
	}

	// Turn values and indices into xt::xarray's
	auto const nnz = values.size();

	coo_xmatrix edge_features{};
	edge_features.values = xt::adapt(std::move(values), {nnz});
	edge_features.indices = decltype(coo_xmatrix::indices)::from_shape({2, nnz});
	xt::row(edge_features.indices, 0) = xt::adapt(std::move(row_indices), {nnz});
	xt::row(edge_features.indices, 1) = xt::adapt(std::move(column_indices), {nnz});
	edge_features.shape = {n_rows, n_cols};

	xmatrix constraint_features = xt::adapt(std::move(biases), {n_rows, 1UL});

	return std::tuple{std::move(edge_features), std::move(constraint_features)};
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

auto MilpBipartite::extract(scip::Model& model, bool /* done */) -> std::optional<MilpBipartiteObs> {
	if (model.get_stage() < SCIP_STAGE_SOLVING) {
		auto [edge_features, constraint_features] = extract_constraints(model, normalize);

		auto variable_features = xmatrix::from_shape({model.variables().size(), MilpBipartiteObs::n_variable_features});
		set_features_for_all_vars(variable_features, model, normalize);

		return MilpBipartiteObs{variable_features, constraint_features, edge_features};
	}
	return {};
}

}  // namespace ecole::observation
