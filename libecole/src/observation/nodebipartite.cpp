#include <array>
#include <cstddef>
#include <limits>

#include <xtensor/xview.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace observation {

using tensor = decltype(NodeBipartiteObs::column_features);
using value_type = tensor::value_type;

static value_type constexpr cste = 5.;
static value_type constexpr nan = std::numeric_limits<value_type>::quiet_NaN();
static auto constexpr n_row_feat = 5;
static auto constexpr n_col_feat =
	11 + scip::enum_size<scip::var_type>::value + scip::enum_size<scip::base_stat>::value;

static value_type get_obj_norm(scip::Model const& model) {
	auto norm = SCIPgetObjNorm(model.get_scip_ptr());
	return norm > 0 ? norm : 1.;
}

static auto extract_col_feat(scip::Model const& model) {
	tensor col_feat{{model.lp_columns().size, n_col_feat}, 0.};

	value_type const n_lps = static_cast<value_type>(SCIPgetNLPs(model.get_scip_ptr()));
	value_type const obj_l2_norm = get_obj_norm(model);

	auto iter = col_feat.begin();
	for (auto const col : model.lp_columns()) {
		auto const var = col.var();
		*(iter++) = static_cast<value_type>(col.lb().has_value());
		*(iter++) = static_cast<value_type>(col.ub().has_value());
		*(iter++) = col.reduced_cost() / obj_l2_norm;
		*(iter++) = col.obj() / obj_l2_norm;
		*(iter++) = col.prim_sol();
		if (var.type_() == SCIP_VARTYPE_CONTINUOUS)
			*(iter++) = 0.;
		else
			*(iter++) = col.prim_sol_frac();
		*(iter++) = static_cast<value_type>(col.is_prim_sol_at_lb());
		*(iter++) = static_cast<value_type>(col.is_prim_sol_at_ub());
		*(iter++) = static_cast<value_type>(col.age()) / (n_lps + cste);
		iter[static_cast<std::size_t>(col.basis_status())] = 1.;
		iter += scip::enum_size<scip::base_stat>::value;
		*(iter++) = var.best_sol_val().value_or(nan);
		*(iter++) = var.avg_sol().value_or(nan);
		iter[static_cast<std::size_t>(var.type_())] = 1.;
		iter += scip::enum_size<scip::var_type>::value;
	}

	// Make sure we iterated over as many element as there are in the tensor
	assert(static_cast<std::size_t>(iter - col_feat.begin()) == col_feat.size());

	return col_feat;
}

/**
 * Number of inequality rows.
 *
 * Row are counted once per right hand side and once per left hand side.
 */
static std::size_t get_n_ineq_rows(scip::Model const& model) {
	std::size_t count = 0;
	for (auto row : model.lp_rows()) {
		count += static_cast<std::size_t>(row.rhs().has_value());
		count += static_cast<std::size_t>(row.lhs().has_value());
	}
	return count;
}

static auto extract_row_feat(scip::Model const& model) {
	tensor row_feat{{get_n_ineq_rows(model), n_row_feat}, 0.};

	value_type const n_lps = static_cast<value_type>(SCIPgetNLPs(model.get_scip_ptr()));
	value_type const obj_l2_norm = get_obj_norm(model);

	auto extract_row = [n_lps, obj_l2_norm](auto& iter, auto const row, bool const lhs) {
		value_type const sign = lhs ? -1. : 1.;
		value_type row_l2_norm = static_cast<value_type>(row.l2_norm());
		if (row_l2_norm == 0) row_l2_norm = 1.;

		if (lhs) {
			*(iter++) = sign * row.lhs().value() / row_l2_norm;
			*(iter++) = static_cast<value_type>(row.is_at_lhs());
		} else {
			*(iter++) = sign * row.rhs().value() / row_l2_norm;
			*(iter++) = static_cast<value_type>(row.is_at_rhs());
		}
		*(iter++) = static_cast<value_type>(row.age()) / (n_lps + cste);
		*(iter++) = sign * row.obj_cos_sim();
		*(iter++) = sign * row.dual_sol() / (row_l2_norm * obj_l2_norm);
	};

	auto iter_ = row_feat.begin();
	for (auto const row_ : model.lp_rows()) {
		// Rows are counted once per rhs and once per lhs
		if (row_.lhs().has_value()) extract_row(iter_, row_, true);
		if (row_.rhs().has_value()) extract_row(iter_, row_, false);
	}

	// Make sure we iterated over as many element as there are in the tensor
	auto diff = iter_ - row_feat.begin();
	(void)diff;
	assert(static_cast<std::size_t>(iter_ - row_feat.begin()) == row_feat.size());

	return row_feat;
}

/**
 * Number of non zero element in the constraint matrix.
 *
 * Row are counted once per right hand side and once per left hand side.
 */
static auto matrix_nnz(scip::Model const& model) {
	std::size_t nnz = 0;
	for (auto row : model.lp_rows()) {
		auto const row_size = static_cast<std::size_t>(row.n_lp_nonz());
		if (row.lhs().has_value()) nnz += row_size;
		if (row.rhs().has_value()) nnz += row_size;
	}
	return nnz;
}

static utility::coo_matrix<value_type> extract_edge_feat(scip::Model const& model) {
	auto const scip = model.get_scip_ptr();

	using coo_matrix = utility::coo_matrix<value_type>;
	auto const nnz = matrix_nnz(model);
	auto values = decltype(coo_matrix::values)::from_shape({nnz});
	auto indices = decltype(coo_matrix::indices)::from_shape({2, nnz});

	std::size_t i = 0, j = 0;
	for (auto const row : model.lp_rows()) {
		SCIP_COL** const row_cols = SCIProwGetCols(row.value);
		scip::real const* const row_vals = SCIProwGetVals(row.value);
		std::size_t const row_nnz = static_cast<std::size_t>(SCIProwGetNLPNonz(row.value));
		if (row.lhs().has_value()) {
			for (std::size_t k = 0; k < row_nnz; ++k) {
				indices(0, j + k) = i;
				indices(1, j + k) = static_cast<std::size_t>(SCIPcolGetLPPos(row_cols[k]));
				values[j + k] = -row_vals[k];
			}
			j += row_nnz;
			i++;
		}
		if (row.rhs().has_value()) {
			for (std::size_t k = 0; k < row_nnz; ++k) {
				indices(0, j + k) = i;
				indices(1, j + k) = static_cast<std::size_t>(SCIPcolGetLPPos(row_cols[k]));
				values[j + k] = row_vals[k];
			}
			j += row_nnz;
			i++;
		}
	}

	auto const n_rows = get_n_ineq_rows(model);
	auto const n_cols = static_cast<std::size_t>(SCIPgetNLPCols(scip));
	return {values, indices, {n_rows, n_cols}};
}

auto NodeBipartite::obtain_observation(scip::Model const& model)
	-> nonstd::optional<NodeBipartiteObs> {
	if (model.get_stage() == SCIP_STAGE_SOLVING) {
		return NodeBipartiteObs{
			extract_col_feat(model), extract_row_feat(model), extract_edge_feat(model)};
	} else {
		return {};
	}
}

}  // namespace observation
}  // namespace ecole
