#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

#include <scip/scip.h>
#include <scip/struct_lp.h>
#include <xtensor/xview.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/row.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::observation {

namespace {

/*********************
 *  Common helpers   *
 *********************/

using tensor = decltype(NodeBipartiteObs::column_features);
using value_type = tensor::value_type;

value_type constexpr cste = 5.;
value_type constexpr nan = std::numeric_limits<value_type>::quiet_NaN();

scip::real obj_l2_norm(Scip* const scip) noexcept {
	auto const norm = SCIPgetObjNorm(scip);
	return norm > 0 ? norm : 1.;
}

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

auto extract_col_feat(scip::Model& model) {
	auto constexpr n_col_feat = 11 + scip::enum_size_v<scip::var_type> + scip::enum_size_v<scip::base_stat>;
	auto* const scip = model.get_scip_ptr();
	tensor col_feat{{model.lp_columns().size(), n_col_feat}, 0.};

	auto const n_lps = static_cast<value_type>(SCIPgetNLPs(scip));
	value_type const obj_norm = obj_l2_norm(scip);

	auto* iter = col_feat.begin();
	for (auto* const col : model.lp_columns()) {
		auto* const var = SCIPcolGetVar(col);
		*(iter++) = static_cast<value_type>(lower_bound(scip, col).has_value());
		*(iter++) = static_cast<value_type>(upper_bound(scip, col).has_value());
		*(iter++) = SCIPgetColRedcost(scip, col) / obj_norm;
		*(iter++) = SCIPcolGetObj(col) / obj_norm;
		*(iter++) = SCIPcolGetPrimsol(col);
		*(iter++) = feas_frac(scip, var, col).value_or(0.);
		*(iter++) = static_cast<value_type>(is_prim_sol_at_lb(scip, col));
		*(iter++) = static_cast<value_type>(is_prim_sol_at_ub(scip, col));
		*(iter++) = static_cast<value_type>(col->age) / (n_lps + cste);
		iter[static_cast<std::size_t>(SCIPcolGetBasisStatus(col))] = 1.;
		iter += scip::enum_size_v<scip::base_stat>;
		*(iter++) = best_sol_val(scip, var).value_or(nan);
		*(iter++) = avg_sol(scip, var).value_or(nan);
		iter[static_cast<std::size_t>(SCIPvarGetType(var))] = 1.;
		iter += scip::enum_size_v<scip::var_type>;
	}

	// Make sure we iterated over as many element as there are in the tensor
	assert(iter == col_feat.end());

	return col_feat;
}

/***************************************
 *  Row features extraction functions  *
 ***************************************/

scip::real row_l2_norm(scip::Row* const row) noexcept {
	auto const norm = SCIProwGetNorm(row);
	return norm > 0 ? norm : 1.;
}

scip::real obj_cos_sim(Scip* const scip, scip::Row* const row) noexcept {
	auto const norm_prod = SCIProwGetNorm(row) * SCIPgetObjNorm(scip);
	if (SCIPisPositive(scip, norm_prod)) {
		return row->objprod / norm_prod;
	}
	return 0.;
}

/**
 * Number of inequality rows.
 *
 * Row are counted once per right hand side and once per left hand side.
 */
std::size_t n_ineq_rows(scip::Model& model) {
	auto* const scip = model.get_scip_ptr();
	std::size_t count = 0;
	for (auto* row : model.lp_rows()) {
		count += static_cast<std::size_t>(scip::get_unshifted_lhs(scip, row).has_value());
		count += static_cast<std::size_t>(scip::get_unshifted_rhs(scip, row).has_value());
	}
	return count;
}

auto extract_row_feat(scip::Model& model) {
	auto constexpr n_row_feat = 5;
	auto* const scip = model.get_scip_ptr();
	tensor row_feat{{n_ineq_rows(model), n_row_feat}, 0.};

	auto const n_lps = static_cast<value_type>(SCIPgetNLPs(scip));
	value_type const obj_norm = obj_l2_norm(scip);

	auto extract_row = [n_lps, obj_norm, scip](auto& iter, auto const row, bool const lhs) {
		value_type const sign = lhs ? -1. : 1.;
		auto row_norm = static_cast<value_type>(row_l2_norm(row));
		if (lhs) {
			*(iter++) = sign * scip::get_unshifted_lhs(scip, row).value() / row_norm;
			*(iter++) = static_cast<value_type>(scip::is_at_lhs(scip, row));
		} else {
			*(iter++) = sign * scip::get_unshifted_rhs(scip, row).value() / row_norm;
			*(iter++) = static_cast<value_type>(scip::is_at_rhs(scip, row));
		}
		*(iter++) = static_cast<value_type>(SCIProwGetAge(row)) / (n_lps + cste);
		*(iter++) = sign * obj_cos_sim(scip, row);
		*(iter++) = sign * SCIProwGetDualsol(row) / (row_norm * obj_norm);
	};

	auto* iter_ = row_feat.begin();
	for (auto* const row_ : model.lp_rows()) {
		// Rows are counted once per rhs and once per lhs
		if (scip::get_unshifted_lhs(scip, row_).has_value()) {
			extract_row(iter_, row_, true);
		}
		if (scip::get_unshifted_rhs(scip, row_).has_value()) {
			extract_row(iter_, row_, false);
		}
	}

	// Make sure we iterated over as many element as there are in the tensor
	assert(iter_ == row_feat.end());

	return row_feat;
}

/****************************************
 *  Edge features extraction functions  *
 ****************************************/

/**
 * Number of non zero element in the constraint matrix.
 *
 * Row are counted once per right hand side and once per left hand side.
 */
auto matrix_nnz(scip::Model& model) {
	auto* const scip = model.get_scip_ptr();
	std::size_t nnz = 0;
	for (auto* row : model.lp_rows()) {
		auto const row_size = static_cast<std::size_t>(SCIProwGetNLPNonz(row));
		if (scip::get_unshifted_lhs(scip, row).has_value()) {
			nnz += row_size;
		}
		if (scip::get_unshifted_rhs(scip, row).has_value()) {
			nnz += row_size;
		}
	}
	return nnz;
}

utility::coo_matrix<value_type> extract_edge_feat(scip::Model& model) {
	auto* const scip = model.get_scip_ptr();

	using coo_matrix = utility::coo_matrix<value_type>;
	auto const nnz = matrix_nnz(model);
	auto values = decltype(coo_matrix::values)::from_shape({nnz});
	auto indices = decltype(coo_matrix::indices)::from_shape({2, nnz});

	std::size_t i = 0;
	std::size_t j = 0;
	for (auto* const row : model.lp_rows()) {
		auto* const row_cols = SCIProwGetCols(row);
		auto const* const row_vals = SCIProwGetVals(row);
		auto const row_nnz = static_cast<std::size_t>(SCIProwGetNLPNonz(row));
		if (scip::get_unshifted_lhs(scip, row).has_value()) {
			for (std::size_t k = 0; k < row_nnz; ++k) {
				indices(0, j + k) = i;
				indices(1, j + k) = static_cast<std::size_t>(SCIPcolGetLPPos(row_cols[k]));
				values[j + k] = -row_vals[k];
			}
			j += row_nnz;
			i++;
		}
		if (scip::get_unshifted_rhs(scip, row).has_value()) {
			for (std::size_t k = 0; k < row_nnz; ++k) {
				indices(0, j + k) = i;
				indices(1, j + k) = static_cast<std::size_t>(SCIPcolGetLPPos(row_cols[k]));
				values[j + k] = row_vals[k];
			}
			j += row_nnz;
			i++;
		}
	}

	auto const n_rows = n_ineq_rows(model);
	auto const n_cols = static_cast<std::size_t>(SCIPgetNLPCols(scip));
	return {values, indices, {n_rows, n_cols}};
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

auto NodeBipartite::extract(scip::Model& model, bool /* done */) -> std::optional<NodeBipartiteObs> {
	if (model.get_stage() == SCIP_STAGE_SOLVING) {
		return NodeBipartiteObs{extract_col_feat(model), extract_row_feat(model), extract_edge_feat(model)};
	}
	return {};
}

}  // namespace ecole::observation
