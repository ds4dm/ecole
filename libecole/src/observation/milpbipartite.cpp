#include <cmath>
#include <cstddef>
#include <type_traits>
#include <fmt/format.h>

#include <scip/scip.h>
#include <scip/struct_lp.h>
#include <xtensor/xview.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xnorm.hpp>

#include "ecole/observation/milpbipartite.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/type.hpp"
#include "ecole/exception.hpp"

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
   This is done by hand because SCIPgetObjNorm is 
   not available for all stages (need >= SCIP_STAGE_TRANSFORMED) */
scip::real obj_l2_norm(Scip* const scip, scip::Model& model) noexcept {
	scip::real norm = 0.;
    
    if (SCIPgetStage(scip) >= SCIP_STAGE_TRANSFORMED) {
        norm = SCIPgetObjNorm(scip);
    } else {
        // If too early, this must be done by hand
        for (auto* variable: model.variables()) {
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
void set_static_features_for_var(Features&& out, Scip* const scip, scip::Var* const var, 
                                 std::optional<value_type> obj_norm = {}) {
    double objsense = (SCIPgetObjsense(scip) == SCIP_OBJSENSE_MINIMIZE) ? 1. : -1.;
    
	out[idx(VariableFeatures::objective)] = objsense * SCIPvarGetObj(var);
    if (obj_norm.has_value()) {
        out[idx(VariableFeatures::objective)] /= obj_norm.value();
    }
	// On-hot enconding of varaible type
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
    
    auto lower_bound = SCIPvarGetLbLocal(var);
    if (SCIPisInfinity(scip, std::abs(lower_bound))) {
        out[idx(VariableFeatures::has_lower_bound)] = 0.;
        out[idx(VariableFeatures::lower_bound)] = 0.;
    } else {
        out[idx(VariableFeatures::has_lower_bound)] = 1.;
        out[idx(VariableFeatures::lower_bound)] = lower_bound;
    }
        
    auto upper_bound = SCIPvarGetUbLocal(var);
    if (SCIPisInfinity(scip, std::abs(upper_bound))) {
        out[idx(VariableFeatures::has_lower_bound)] = 0.;
        out[idx(VariableFeatures::lower_bound)] = 0.;
    } else {
        out[idx(VariableFeatures::has_upper_bound)] = 1.;
        out[idx(VariableFeatures::upper_bound)] = upper_bound;
    }
}

void set_features_for_all_vars(xmatrix& out, scip::Model& model, bool normalize) {
	auto* const scip = model.get_scip_ptr();

	// Contant reused in every iterations
    auto const obj_norm = normalize ? std::make_optional(obj_l2_norm(scip, model)) : std::nullopt;

	auto const variables = model.variables();
	auto const n_vars = variables.size();
	for (std::size_t var_idx = 0; var_idx < n_vars; ++var_idx) {
		auto* const variable = variables[var_idx];
		auto features = xt::row(out, static_cast<std::ptrdiff_t>(var_idx));
        set_static_features_for_var(features, scip, variable, obj_norm);
	}
}


/****************************************
 *  Constraint extraction functions     *
 ****************************************/

scip::real cons_l2_norm(std::vector<scip::real> constraint_coefs) noexcept {
    std::vector<std::size_t> shape = {constraint_coefs.size()};
    auto xt_constraint_coefs = xt::adapt(constraint_coefs, shape);
    
	auto norm = xt::norm_l2(xt_constraint_coefs)();
	return norm > 0. ? norm : 1.;
}
    
/**
 * Obtains the variables involved in a linear constraint and their coefficients in the constraint
 */
auto get_constraint_coefs(Scip* const scip, scip::Cons* const constraint) -> 
    std::optional<std::tuple<std::vector<scip::Var*>, std::vector<scip::real>, std::optional<scip::real>, std::optional<scip::real>>> {
	SCIP_Bool success = false;
    int n_constraint_variables;
    int n_active_variables;
    scip::real constant_offset = 0;
    int requiredsize = 0;
    
    // Find how many active variables and constraint variables there are (for allocation)
    scip::call(SCIPgetConsNVars, scip, constraint, &n_constraint_variables, &success);
    if (!success) return {};
    n_active_variables = SCIPgetNVars(scip);
    
    // Allocate buffers large enough to hold future variables and coefficients
    int buffer_size = std::max(n_constraint_variables, n_active_variables);
    auto* variables = new scip::Var*[buffer_size];
    auto* coefficients = new scip::real[buffer_size];
    
    // Get the variables and their coefficients in the constraint
    scip::call(SCIPgetConsVars, scip, constraint, variables, buffer_size, &success);
    if (!success) return {};
    scip::call(SCIPgetConsVals, scip, constraint, coefficients, buffer_size, &success);
    if (!success) return {};
    
    // If we are in SCIP_STAGE_TRANSFORMED or later, the variables in the constraint might be inactive
    // Re-express the coefficients in terms of active variables
    if (SCIPgetStage(scip) >= SCIP_STAGE_TRANSFORMED) {
        scip::call(SCIPgetProbvarLinearSum, scip, variables, coefficients, &n_constraint_variables, 
                   buffer_size, &constant_offset, &requiredsize, true);
    }
    
    // Copy the relevant sections of the buffers to output vectors
    std::vector<scip::Var*> output_variables;
    std::vector<scip::real> output_coefficients;
    output_variables.insert(output_variables.end(), variables, &variables[n_constraint_variables]); 
    output_coefficients.insert(output_coefficients.end(), coefficients, &coefficients[n_constraint_variables]);
    
    // Obtain the left hand side
    std::optional<scip::real> lhs;
    scip::real lhs_value = SCIPconsGetLhs(scip, constraint, &success);
    if (success && !SCIPisInfinity(scip, std::abs(lhs_value))) {
        lhs = lhs_value - constant_offset;
    }
    else lhs = std::nullopt;
    
    std::optional<scip::real> rhs;
    scip::real rhs_value = SCIPconsGetRhs(scip, constraint, &success);
    if (success && !SCIPisInfinity(scip, std::abs(rhs_value))) {
        rhs = rhs_value - constant_offset;
    }
    else rhs = std::nullopt;
    
    // Free the buffers
    delete variables;
    delete coefficients;
    
    return std::make_tuple(output_variables, output_coefficients, lhs, rhs);
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
    for(std::size_t cons_idx = 0; cons_idx < std::size(constraints); ++cons_idx) {
        auto const constraint = constraints[cons_idx];
        auto const constraint_data = get_constraint_coefs(scip, constraint);
        if (constraint_data.has_value()) { // Constraint must be linear
            std::vector<scip::Var*> constraint_vars;
            std::vector<scip::real> constraint_coefs;
            std::optional<scip::real> lhs, rhs;
            std::tie(constraint_vars, constraint_coefs, lhs, rhs) = constraint_data.value();
            scip::real constraint_norm = 0;
            if (normalize)
                constraint_norm = cons_l2_norm(constraint_coefs);
            
            // Inequality has a left hand side?
            if (lhs.has_value()) {
                for(std::size_t cons_var_idx = 0; cons_var_idx < std::size(constraint_vars); ++cons_var_idx) {
                    value_type value = constraint_coefs[cons_var_idx];
                    int var_idx = SCIPvarGetProbindex(constraint_vars[cons_var_idx]);
                    
                    values.push_back(-value);
                    row_indices.push_back(n_rows);
                    column_indices.push_back(static_cast<std::size_t>(var_idx));
                }
                if (normalize) 
                    biases.push_back(-lhs.value() / constraint_norm); 
                else 
                    biases.push_back(-lhs.value());
                n_rows++;
            }
            // Inequality has a right hand side?
            if (rhs.has_value()) {
                for(std::size_t cons_var_idx = 0; cons_var_idx < std::size(constraint_vars); ++cons_var_idx) {
                    value_type value = constraint_coefs[cons_var_idx];
                    int var_idx = SCIPvarGetProbindex(constraint_vars[cons_var_idx]);
                    
                    values.push_back(value);
                    row_indices.push_back(n_rows);
                    column_indices.push_back(static_cast<std::size_t>(var_idx));
                }
                if (normalize) 
                    biases.push_back(rhs.value() / constraint_norm); 
                else 
                    biases.push_back(rhs.value());
                n_rows++;
            }
        } else {
            throw Exception(fmt::format("Constraint {} cannot be expressed as a single linear constraint (type \"{}\"), MilpBipartite observation cannot be extracted.", 
                            cons_idx, SCIPconshdlrGetName(SCIPconsGetHdlr(constraint))));
        }
    }
    
    // Turn values and indices into xt::xarray's
    auto nnz = std::size(values);
    
    auto edge_values = xt::adapt(values, {nnz});
	auto edge_indices = decltype(coo_xmatrix::indices)::from_shape({2, nnz});
    xt::row(edge_indices, 0) = xt::adapt(row_indices, {nnz});
    xt::row(edge_indices, 1) = xt::adapt(column_indices, {nnz});
    coo_xmatrix edge_features = {edge_values, edge_indices, {n_rows, n_cols}};
    
    std::vector<std::size_t> constraint_features_shape = {n_rows, 1};
    xmatrix constraint_features = xt::adapt(biases, constraint_features_shape);
    
    return std::make_tuple(edge_features, constraint_features);
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

auto MilpBipartite::extract(scip::Model& model, bool /* done */) -> std::optional<MilpBipartiteObs> {
	if (model.get_stage() < SCIP_STAGE_SOLVING) {
        coo_xmatrix edge_features;
        xmatrix constraint_features;
        std::tie(edge_features, constraint_features) = extract_constraints(model, normalize);

        auto variable_features = xmatrix::from_shape({model.variables().size(), MilpBipartiteObs::n_variable_features});
        set_features_for_all_vars(variable_features, model, normalize);

        return MilpBipartiteObs{variable_features, constraint_features, edge_features};
	}
	return {};
}

}  // namespace ecole::observation
