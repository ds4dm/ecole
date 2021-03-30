#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <iostream>
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
#include "ecole/scip/utils.hpp"
#include "ecole/exception.hpp"

namespace ecole::observation {

namespace {

/*********************
 *  Common helpers   *
 *********************/

using xmatrix = decltype(MilpBipartiteObs::variable_features);
using value_type = xmatrix::value_type;

using VariableFeatures = MilpBipartiteObs::VariableFeatures;
using ConstraintFeatures = MilpBipartiteObs::ConstraintFeatures;

/******************************************
 *  Column features extraction functions  *
 ******************************************/
    
scip::real obj_l2_norm(Scip* const scip) noexcept {
	auto const norm = SCIPgetObjNorm(scip);
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
}

void set_features_for_all_vars(xmatrix& out, scip::Model& model, bool normalize) {
	auto* const scip = model.get_scip_ptr();

	// Contant reused in every iterations
    auto const obj_norm = normalize ? std::make_optional(obj_l2_norm(scip)) : std::nullopt;

	auto const variables = model.variables();
	auto const n_vars = variables.size();
	for (std::size_t var_idx = 0; var_idx < n_vars; ++var_idx) {
		auto* const variable = variables[var_idx];
		auto features = xt::row(out, static_cast<std::ptrdiff_t>(var_idx));
        set_static_features_for_var(features, scip, variable, obj_norm);
	}
}

/***************************************
 *  Row features extraction functions  *
 ***************************************/
    
/**
 * Number of inequality constraints.
 *
 * Constraints are counted once per right hand side and once per left hand side.
 */
std::size_t n_ineq_constraints(scip::Model& model) {
	auto* const scip = model.get_scip_ptr();
    std::size_t count = 0;

	for (auto* const constraint: model.constraints()) {
        auto lhs = scip::cons_get_lhs(scip, constraint);
        auto rhs = scip::cons_get_rhs(scip, constraint);
        if (lhs.has_value() && !SCIPisInfinity(scip, std::abs(lhs.value())))
            ++count;
        if (rhs.has_value() && !SCIPisInfinity(scip, std::abs(rhs.value())))
            ++count;
	}
	return count;
}

scip::real cons_l2_norm(Scip* const scip, scip::Cons* const cons) noexcept {
    auto const values = scip::get_vals_linear(scip, cons);
    std::vector<std::size_t> shape = {values.size()};
    xt::xarray<scip::real> xt_values = xt::adapt(values.data(), values.size(), xt::no_ownership(), shape);
    
	auto norm = xt::norm_l2(xt_values)();
	return norm > 0. ? norm : 1.;
}


template <typename Features>
void set_static_features_for_lhs_constraint(Features&& out, value_type lhs, std::optional<value_type> cons_norm = {}) {
	out[idx(ConstraintFeatures::bias)] = -lhs;
    if (cons_norm.has_value()) {
        out[idx(ConstraintFeatures::bias)] /= cons_norm.value();
    }
}

template <typename Features>
void set_static_features_for_rhs_constraint(Features&& out, value_type rhs, std::optional<value_type> cons_norm = {}) {
	out[idx(ConstraintFeatures::bias)] = rhs;
    if (cons_norm.has_value()) {
        out[idx(ConstraintFeatures::bias)] /= cons_norm.value();
    }
}

auto set_features_for_all_cons(xmatrix& out, scip::Model& model, bool normalize) {
	auto* const scip = model.get_scip_ptr();

	auto const constraints = model.constraints();
	auto const n_cons = constraints.size();
	for (std::size_t cons_idx = 0; cons_idx < n_cons; ++cons_idx) {
		auto* const constraint = constraints[cons_idx];
		auto const cons_norm = normalize ? std::make_optional(static_cast<value_type>(cons_l2_norm(scip, constraint))) : std::nullopt;
		auto features = xt::row(out, static_cast<std::ptrdiff_t>(cons_idx));

		// Rows are counted once per rhs and once per lhs
        auto lhs = scip::cons_get_lhs(scip, constraint);
        if (lhs.has_value() && !SCIPisInfinity(scip, std::abs(lhs.value()))) {
			set_static_features_for_lhs_constraint(features, lhs.value(), cons_norm);
		}
        auto rhs = scip::cons_get_rhs(scip, constraint);
        if (rhs.has_value() && !SCIPisInfinity(scip, std::abs(rhs.value()))) {
            set_static_features_for_rhs_constraint(features, rhs.value(), cons_norm);
		}
	}
}


/****************************************
 *  Edge features extraction functions  *
 ****************************************/
 
auto get_constraint_coefs(Scip* const scip, scip::Cons* const constraint) -> std::optional<nonstd::span<scip::real const>> {
    int n_vars;
	SCIP_Bool success = false;
    
    scip::call(SCIPgetConsNVars, scip, constraint, &n_vars, &success);
    if (success) {
        scip::real* values = new SCIP_Real[n_vars];
        scip::call(SCIPgetConsVals, scip, constraint, values, n_vars, &success);
        nonstd::span<scip::real const> output = {values, static_cast<std::size_t>(n_vars)};
        return output;
    }
    return {};
}

template <typename value_type>
utility::coo_matrix<value_type> extract_edge_features(scip::Model& model) {
	auto* const scip = model.get_scip_ptr();
    auto const constraints = model.constraints();
    auto n_cons = static_cast<std::size_t>(SCIPgetNConss(scip));
    auto n_vars = static_cast<std::size_t>(SCIPgetNVars(scip));

	using coo_matrix = utility::coo_matrix<value_type>;
    std::vector<value_type> raw_values;
    std::vector<std::size_t> column_indices;
    std::vector<std::size_t> row_indices;
    std::size_t n_rows = 0;
    
    for(std::size_t cons_idx = 0; cons_idx < n_cons; ++cons_idx) {
        auto const constraint = constraints[cons_idx];
        auto const constraint_values = get_constraint_coefs(scip, constraint);
        auto n_cons_vars = std::size(constraint_values.value());
        if (constraint_values.has_value()) {
            SCIP_Bool success = false;
            auto cons_vars = new scip::Var*[n_cons_vars];
            scip::call(SCIPgetConsVars, scip, constraint, cons_vars, static_cast<int>(n_cons_vars), &success);
            
            // Inequality has a left hand side?
            auto lhs = scip::cons_get_lhs(scip, constraint);
            if (lhs.has_value() && !SCIPisInfinity(scip, std::abs(lhs.value()))) {
                for(std::size_t cons_var_idx = 0; cons_var_idx < n_cons_vars; ++cons_var_idx) {
                    value_type value = constraint_values.value()[cons_var_idx]; 
                    int var_idx = SCIPvarGetIndex(cons_vars[cons_var_idx]);
                    
                    if (value != 0) {
                        raw_values.push_back(-value);
                        row_indices.push_back(n_rows);
                        column_indices.push_back(static_cast<std::size_t>(var_idx));
                    }
                }
                n_rows++;
            }
            // Inequality has a right hand side?
            auto rhs = scip::cons_get_rhs(scip, constraint);
            if (rhs.has_value() && !SCIPisInfinity(scip, std::abs(rhs.value()))) {
                for(std::size_t cons_var_idx = 0; cons_var_idx < n_cons_vars; ++cons_var_idx) {
                    value_type value = constraint_values.value()[cons_var_idx]; 
                    int var_idx = SCIPvarGetProbindex(cons_vars[cons_var_idx]);
                    
                    if (value != 0) {
                        raw_values.push_back(value);
                        row_indices.push_back(n_rows);
                        column_indices.push_back(static_cast<std::size_t>(var_idx));
                    }
                }
                n_rows++;
            }
        } else {
            throw Exception(fmt::format("Constraint {} is non-linear (type \"{}\"), MilpBipartite observation cannot be extracted.", 
                            cons_idx, SCIPconshdlrGetName(SCIPconsGetHdlr(constraint))));
        }   
    }
    
    // Turn values and indices into xt::xarray's
    auto nnz = std::size(raw_values);
    auto values = xt::adapt(raw_values, {nnz});
	auto indices = decltype(coo_matrix::indices)::from_shape({2, nnz});
    xt::row(indices, 0) = xt::adapt(row_indices, {nnz});
    xt::row(indices, 1) = xt::adapt(column_indices, {nnz});
    
    return {values, indices, {n_rows, n_vars}};
}

/****************************************
 *  General functions  *
 ****************************************/

auto extract_observation(scip::Model& model, bool normalize) -> MilpBipartiteObs {
	auto obs = MilpBipartiteObs{
		xmatrix::from_shape({model.variables().size(), MilpBipartiteObs::n_variable_features}),
		xmatrix::from_shape({n_ineq_constraints(model), MilpBipartiteObs::n_constraint_features}),
		extract_edge_features<value_type>(model),
	};
	set_features_for_all_vars(obs.variable_features, model, normalize);
	set_features_for_all_cons(obs.constraint_features, model, normalize);
	return obs;
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

auto MilpBipartite::extract(scip::Model& model, bool /* done */) -> std::optional<MilpBipartiteObs> {
// 	if (model.get_stage() < SCIP_STAGE_SOLVING) {
    if (model.get_stage() == SCIP_STAGE_SOLVING) {
        return extract_observation(model, normalize);
	}
	return {};
}

}  // namespace ecole::observation
