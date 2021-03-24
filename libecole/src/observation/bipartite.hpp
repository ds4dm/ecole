#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>

#include <scip/scip.h>
#include <scip/struct_lp.h>
#include <xtensor/xview.hpp>


#include <optional>

#include <xtensor/xtensor.hpp>

#include "ecole/observation/abstract.hpp"
#include "ecole/utility/sparse-matrix.hpp"

#include "ecole/scip/model.hpp"
#include "ecole/scip/row.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::observation {

/******************************************
 *  Column features extraction functions  *
 ******************************************/

/** Convert an enum to its underlying index. */
template <typename E> constexpr auto idx(E e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

template <typename Features, typename ColumnFeatures>
void set_static_features_for_col(Features&& out, scip::Var* const var, scip::Col* const col, typename Features::value_type obj_norm) {
// 	out[idx(ColumnFeatures::objective)] = SCIPcolGetObj(col) / obj_norm;
// 	// On-hot enconding of varaible type
// 	out[idx(ColumnFeatures::is_type_binary)] = 0.;
// 	out[idx(ColumnFeatures::is_type_integer)] = 0.;
// 	out[idx(ColumnFeatures::is_type_implicit_integer)] = 0.;
// 	out[idx(ColumnFeatures::is_type_continuous)] = 0.;
// 	switch (SCIPvarGetType(var)) {
// 	case SCIP_VARTYPE_BINARY:
// 		out[idx(ColumnFeatures::is_type_binary)] = 1.;
// 		break;
// 	case SCIP_VARTYPE_INTEGER:
// 		out[idx(ColumnFeatures::is_type_integer)] = 1.;
// 		break;
// 	case SCIP_VARTYPE_IMPLINT:
// 		out[idx(ColumnFeatures::is_type_implicit_integer)] = 1.;
// 		break;
// 	case SCIP_VARTYPE_CONTINUOUS:
// 		out[idx(ColumnFeatures::is_type_continuous)] = 1.;
// 		break;
// 	default:
// 		assert(false);  // All enum cases must be handled
// 	}
}

/***************************************
 *  Row features extraction functions  *
 ***************************************/

inline scip::real obj_l2_norm(Scip* const scip) noexcept {
	auto const norm = SCIPgetObjNorm(scip);
	return norm > 0 ? norm : 1.;
}

inline scip::real row_l2_norm(scip::Row* const row) noexcept {
	auto const norm = SCIProwGetNorm(row);
	return norm > 0 ? norm : 1.;
}

inline scip::real obj_cos_sim(Scip* const scip, scip::Row* const row) noexcept {
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
inline std::size_t n_ineq_rows(scip::Model& model) {
	auto* const scip = model.get_scip_ptr();
	std::size_t count = 0;
	for (auto* row : model.lp_rows()) {
		count += static_cast<std::size_t>(scip::get_unshifted_lhs(scip, row).has_value());
		count += static_cast<std::size_t>(scip::get_unshifted_rhs(scip, row).has_value());
	}
	return count;
}

template <typename Features, typename RowFeatures>
void set_static_features_for_lhs_row(Features&& out, Scip* const scip, scip::Row* const row, typename Features::value_type row_norm) {
// 	out[idx(RowFeatures::bias)] = -1. * scip::get_unshifted_lhs(scip, row).value() / row_norm;
// 	out[idx(RowFeatures::objective_cosine_similarity)] = -1 * obj_cos_sim(scip, row);
}

template <typename Features, typename RowFeatures>
void set_static_features_for_rhs_row(Features&& out, Scip* const scip, scip::Row* const row, typename Features::value_type row_norm) {
// 	out[idx(RowFeatures::bias)] = scip::get_unshifted_rhs(scip, row).value() / row_norm;
// 	out[idx(RowFeatures::objective_cosine_similarity)] = obj_cos_sim(scip, row);
}

/****************************************
 *  Edge features extraction functions  *
 ****************************************/

/**
 * Number of non zero element in the constraint matrix.
 *
 * Row are counted once per right hand side and once per left hand side.
 */
inline auto matrix_nnz(scip::Model& model) {
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

template <typename value_type>
utility::coo_matrix<value_type> extract_edge_features(scip::Model& model) {
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

}  // namespace ecole::observation
