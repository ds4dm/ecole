#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>

#include <scip/scip.h>
#include <scip/struct_lp.h>
#include <xtensor/xview.hpp>

#include "ecole/observation/milpbipartite.hpp"
#include "observation/bipartite.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/row.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::observation {

namespace {

/*********************
 *  Common helpers   *
 *********************/

using xmatrix = decltype(MilpBipartiteObs::column_features);
using value_type = xmatrix::value_type;

using ColumnFeatures = MilpBipartiteObs::ColumnFeatures;
using RowFeatures = MilpBipartiteObs::RowFeatures;

/******************************************
 *  Column features extraction functions  *
 ******************************************/

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
        set_static_features_for_col<decltype(features), ColumnFeatures>(features, var, col, obj_norm);
	}
}

/***************************************
 *  Row features extraction functions  *
 ***************************************/

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
			set_static_features_for_lhs_row<decltype(features), RowFeatures>(features, scip, row, row_norm);
		}
		if (scip::get_unshifted_rhs(scip, row).has_value()) {
            set_static_features_for_rhs_row<decltype(features), RowFeatures>(features, scip, row, row_norm);
		}
	}
}

auto extract_observation(scip::Model& model, bool normalize) -> MilpBipartiteObs {
	auto obs = MilpBipartiteObs{
		xmatrix::from_shape({model.lp_columns().size(), MilpBipartiteObs::n_column_features}),
		xmatrix::from_shape({n_ineq_rows(model), MilpBipartiteObs::n_row_features}),
		extract_edge_features(model),
	};
	set_features_for_all_cols(obs.column_features, model, true);
	set_features_for_all_rows(obs.row_features, model, true);
	return obs;
}

}  // namespace

/*************************************
 *  Observation extracting function  *
 *************************************/

auto MilpBipartite::extract(scip::Model& model, bool /* done */) -> std::optional<MilpBipartiteObs> {
	if (model.get_stage() == SCIP_STAGE_SOLVING) {
		return extract_observation(model, normalize);
	}
	return {};
}

}  // namespace ecole::observation
