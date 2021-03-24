#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <optional>

#include <scip/scip.h>
#include <scip/struct_lp.h>
#include <xtensor/xview.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "observation/bipartite.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/row.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::observation {

namespace {

/*********************
 *  Common helpers   *
 *********************/

using xmatrix = decltype(NodeBipartiteObs::column_features);
using value_type = xmatrix::value_type;

using ColumnFeatures = NodeBipartiteObs::ColumnFeatures;
using RowFeatures = NodeBipartiteObs::RowFeatures;

value_type constexpr cste = 5.;
value_type constexpr nan = std::numeric_limits<value_type>::quiet_NaN();

/******************************************
 *  Column features extraction functions  *
 ******************************************/

std::optional<scip::real> upper_bound(Scip* const scip, scip::Col* const col) noexcept {
	auto const ub_val = SCIPcolGetUb(col);
	if (SCIPisInfinity(scip, std::abs(ub_val))) {
		return {};
	}
	return ub_val;
}

std::optional<scip::real> lower_bound(Scip* const scip, scip::Col* const col) noexcept {
	auto const lb_val = SCIPcolGetLb(col);
	if (SCIPisInfinity(scip, std::abs(lb_val))) {
		return {};
	}
	return lb_val;
}

bool is_prim_sol_at_lb(Scip* const scip, scip::Col* const col) noexcept {
	auto const lb_val = lower_bound(scip, col);
	if (lb_val) {
		return SCIPisEQ(scip, SCIPcolGetPrimsol(col), lb_val.value());
	}
	return false;
}

bool is_prim_sol_at_ub(Scip* const scip, scip::Col* const col) noexcept {
	auto const ub_val = upper_bound(scip, col);
	if (ub_val) {
		return SCIPisEQ(scip, SCIPcolGetPrimsol(col), ub_val.value());
	}
	return false;
}

std::optional<scip::real> best_sol_val(Scip* const scip, scip::Var* const var) noexcept {
	auto* const sol = SCIPgetBestSol(scip);
	if (sol != nullptr) {
		return SCIPgetSolVal(scip, sol, var);
	}
	return {};
}

std::optional<scip::real> avg_sol(Scip* const scip, scip::Var* const var) noexcept {
	if (SCIPgetBestSol(scip) != nullptr) {
		return SCIPvarGetAvgSol(var);
	}
	return {};
}

std::optional<scip::real> feas_frac(Scip* const scip, scip::Var* const var, scip::Col* const col) noexcept {
	if (SCIPvarGetType(var) == SCIP_VARTYPE_CONTINUOUS) {
		return {};
	}
	return SCIPfeasFrac(scip, SCIPcolGetPrimsol(col));
}

template <typename Features>
void set_dynamic_features_for_col(
	Features&& out,
	Scip* const scip,
	scip::Var* const var,
	scip::Col* const col,
	std::optional<value_type> obj_norm = {},
	std::optional<value_type> n_lps = {}) {
	out[idx(ColumnFeatures::has_lower_bound)] = static_cast<value_type>(lower_bound(scip, col).has_value());
	out[idx(ColumnFeatures::has_upper_bound)] = static_cast<value_type>(upper_bound(scip, col).has_value());
	out[idx(ColumnFeatures::normed_reduced_cost)] = SCIPgetColRedcost(scip, col);
    if (obj_norm.has_value()) {
        out[idx(ColumnFeatures::normed_reduced_cost)] /= obj_norm.value();
    }
	out[idx(ColumnFeatures::solution_value)] = SCIPcolGetPrimsol(col);
	out[idx(ColumnFeatures::solution_frac)] = feas_frac(scip, var, col).value_or(0.);
	out[idx(ColumnFeatures::is_solution_at_lower_bound)] = static_cast<value_type>(is_prim_sol_at_lb(scip, col));
	out[idx(ColumnFeatures::is_solution_at_upper_bound)] = static_cast<value_type>(is_prim_sol_at_ub(scip, col));
	out[idx(ColumnFeatures::scaled_age)] = static_cast<value_type>(SCIPcolGetAge(col));
    if (n_lps.has_value()) {
        out[idx(ColumnFeatures::scaled_age)] /= (n_lps.value() + cste);
    }
	out[idx(ColumnFeatures::incumbent_value)] = best_sol_val(scip, var).value_or(nan);
	out[idx(ColumnFeatures::average_incumbent_value)] = avg_sol(scip, var).value_or(nan);
	// On-hot encoding
	out[idx(ColumnFeatures::is_basis_lower)] = 0.;
	out[idx(ColumnFeatures::is_basis_basic)] = 0.;
	out[idx(ColumnFeatures::is_basis_upper)] = 0.;
	out[idx(ColumnFeatures::is_basis_zero)] = 0.;
	switch (SCIPcolGetBasisStatus(col)) {
	case SCIP_BASESTAT_LOWER:
		out[idx(ColumnFeatures::is_basis_lower)] = 1.;
		break;
	case SCIP_BASESTAT_BASIC:
		out[idx(ColumnFeatures::is_basis_basic)] = 1.;
		break;
	case SCIP_BASESTAT_UPPER:
		out[idx(ColumnFeatures::is_basis_upper)] = 1.;
		break;
	case SCIP_BASESTAT_ZERO:
		out[idx(ColumnFeatures::is_basis_zero)] = 1.;
		break;
	default:
		assert(false);  // All enum cases must be handled
	}
}

void set_features_for_all_cols(xmatrix& out, scip::Model& model, bool const update_static, bool normalize) {
	auto* const scip = model.get_scip_ptr();

    // Contant reused in every iterations
    auto const n_lps = normalize ? std::make_optional(static_cast<value_type>(SCIPgetNLPs(scip))) : std::nullopt;
    auto const obj_norm = normalize ? std::make_optional(obj_l2_norm(scip)) : std::nullopt;

	auto const columns = model.lp_columns();
	auto const n_columns = columns.size();
	for (std::size_t col_idx = 0; col_idx < n_columns; ++col_idx) {
		auto* const col = columns[col_idx];
		auto* const var = SCIPcolGetVar(col);
		auto features = xt::row(out, static_cast<std::ptrdiff_t>(col_idx));
		if (update_static) {
			set_static_features_for_col<decltype(features)&, ColumnFeatures>(features, var, col, obj_norm);
		}
		set_dynamic_features_for_col(features, scip, var, col, obj_norm, n_lps);
	}
}

/***************************************
 *  Row features extraction functions  *
 ***************************************/

template <typename Features>
void set_dynamic_features_for_lhs_row(
	Features&& out,
	Scip* const scip,
	scip::Row* const row,
	std::optional<value_type> row_norm = {},
	std::optional<value_type> obj_norm = {},
	std::optional<value_type> n_lps = {}) {
	out[idx(RowFeatures::is_tight)] = static_cast<value_type>(scip::is_at_rhs(scip, row));
	out[idx(RowFeatures::dual_solution_value)] = -1. * SCIProwGetDualsol(row);
	out[idx(RowFeatures::scaled_age)] = static_cast<value_type>(SCIProwGetAge(row));
    if (row_norm.has_value() && obj_norm.has_value()) {
        out[idx(RowFeatures::dual_solution_value)] /= (row_norm.value() * obj_norm.value());
    }
    if (n_lps.has_value()) {
        out[idx(RowFeatures::scaled_age)] /= (n_lps.value() + cste);
    }
}
    
template <typename Features>
void set_dynamic_features_for_rhs_row(
	Features&& out,
	Scip* const scip,
	scip::Row* const row,
	std::optional<value_type> row_norm = {},
	std::optional<value_type> obj_norm = {},
	std::optional<value_type> n_lps = {}) {
	out[idx(RowFeatures::is_tight)] = static_cast<value_type>(scip::is_at_rhs(scip, row));
	out[idx(RowFeatures::dual_solution_value)] = SCIProwGetDualsol(row);
	out[idx(RowFeatures::scaled_age)] = static_cast<value_type>(SCIProwGetAge(row));
    if (row_norm.has_value() && obj_norm.has_value()) {
        out[idx(RowFeatures::dual_solution_value)] /= (row_norm.value() * obj_norm.value());
    }
    if (n_lps.has_value()) {
        out[idx(RowFeatures::scaled_age)] /= (n_lps.value() + cste);
    }
}


auto set_features_for_all_rows(xmatrix& out, scip::Model& model, bool const update_static, bool normalize) {
	auto* const scip = model.get_scip_ptr();

    auto const n_lps = normalize ? std::make_optional(static_cast<value_type>(SCIPgetNLPs(scip))) : std::nullopt;
    auto const obj_norm = normalize ? std::make_optional(obj_l2_norm(scip)) : std::nullopt;

	auto const rows = model.lp_rows();
	auto const n_rows = rows.size();
	for (std::size_t row_idx = 0; row_idx < n_rows; ++row_idx) {
		auto* const row = rows[row_idx];
		auto const row_norm = normalize ? std::make_optional(static_cast<value_type>(row_l2_norm(row))) : std::nullopt;
		auto features = xt::row(out, static_cast<std::ptrdiff_t>(row_idx));

		// Rows are counted once per rhs and once per lhs
		if (scip::get_unshifted_lhs(scip, row).has_value()) {
			if (update_static) {
				set_static_features_for_lhs_row<decltype(features)&, RowFeatures>(features, scip, row, row_norm);
			}
			set_dynamic_features_for_lhs_row(features, scip, row, row_norm, obj_norm, n_lps);
		}
		if (scip::get_unshifted_rhs(scip, row).has_value()) {
			if (update_static) {
				set_static_features_for_rhs_row<decltype(features)&, RowFeatures>(features, scip, row, row_norm);
			}
			set_dynamic_features_for_rhs_row(features, scip, row, row_norm, obj_norm, n_lps);
		}
	}
}

auto is_on_root_node(scip::Model& model) -> bool {
	auto* const scip = model.get_scip_ptr();
	return SCIPgetCurrentNode(scip) == SCIPgetRootNode(scip);
}

auto extract_observation_fully(scip::Model& model, bool normalize) -> NodeBipartiteObs {
	auto obs = NodeBipartiteObs{
		xmatrix::from_shape({model.lp_columns().size(), NodeBipartiteObs::n_column_features}),
		xmatrix::from_shape({n_ineq_rows(model), NodeBipartiteObs::n_row_features}),
		extract_edge_features<value_type>(model),
	};
	set_features_for_all_cols(obs.column_features, model, true, normalize);
	set_features_for_all_rows(obs.row_features, model, true, normalize);
	return obs;
}

auto extract_observation_from_cache(scip::Model& model, NodeBipartiteObs obs, bool normalize) -> NodeBipartiteObs {
	set_features_for_all_cols(obs.column_features, model, false, normalize);
	set_features_for_all_rows(obs.row_features, model, false, normalize);
	return obs;
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

auto NodeBipartite::before_reset(scip::Model & /* model */) -> void {
	cache_computed = false;
}

auto NodeBipartite::extract(scip::Model& model, bool /* done */) -> std::optional<NodeBipartiteObs> {
	if (model.get_stage() == SCIP_STAGE_SOLVING) {
		if (use_cache) {
			if (is_on_root_node(model)) {
				the_cache = extract_observation_fully(model, normalize);
				cache_computed = true;
				return the_cache;
			}
			if (cache_computed) {
				return extract_observation_from_cache(model, the_cache, normalize);
			}
		}
		return extract_observation_fully(model, normalize);
	}
	return {};
}

}  // namespace ecole::observation
