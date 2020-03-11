#include <array>
#include <cstddef>
#include <limits>

#include <xtensor/xview.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace observation {

auto NodeBipartite::clone() const -> std::unique_ptr<Base> {
	return std::make_unique<NodeBipartite>(*this);
}

using tensor = decltype(NodeBipartiteObs::col_feat);
using value_type = tensor::value_type;

constexpr static auto nan = std::numeric_limits<value_type>::quiet_NaN();
constexpr static auto n_row_feat = 13 + scip::enum_size<scip::base_stat>::value;
constexpr static auto n_col_feat =
	11 + scip::enum_size<scip::var_type>::value + scip::enum_size<scip::base_stat>::value;

static auto extract_col_feat(scip::Model const& model) {
	tensor col_feat{{model.lp_columns().size, n_col_feat}, 0.};

	auto iter = col_feat.begin();
	for (auto col : model.lp_columns()) {
		*(iter++) = static_cast<value_type>(col.lb().value_or(nan));
		*(iter++) = static_cast<value_type>(col.ub().value_or(nan));
		*(iter++) = static_cast<value_type>(col.reduced_cost());
		*(iter++) = static_cast<value_type>(col.obj());
		*(iter++) = static_cast<value_type>(col.prim_sol());
		*(iter++) = static_cast<value_type>(col.prim_sol_frac());
		*(iter++) = static_cast<value_type>(col.is_prim_sol_at_lb());
		*(iter++) = static_cast<value_type>(col.is_prim_sol_at_ub());
		*(iter++) = static_cast<value_type>(col.age());
		iter[static_cast<std::size_t>(col.basis_status())] = 1.;
		iter += scip::enum_size<scip::base_stat>::value;
		auto const var = col.var();
		*(iter++) = static_cast<value_type>(var.best_sol_val().value_or(nan));
		*(iter++) = static_cast<value_type>(var.avg_sol().value_or(nan));
		iter[static_cast<std::size_t>(var.type_())] = 1.;
		iter += scip::enum_size<scip::var_type>::value;
	}

	// Make sure we iterated over as many element as there are in the tensor
	assert(static_cast<std::size_t>(iter - col_feat.begin()) == col_feat.size());

	return col_feat;
}

static auto extract_row_feat(scip::Model const& model) {
	tensor row_feat{{model.lp_rows().size, n_row_feat}, 0.};

	auto iter = row_feat.begin();
	auto count = 0;
	for (auto row : model.lp_rows()) {
		*(iter++) = static_cast<value_type>(row.rhs().value_or(nan));
		*(iter++) = static_cast<value_type>(row.lhs().value_or(nan));
		*(iter++) = static_cast<value_type>(row.n_lp_nonz());
		*(iter++) = static_cast<value_type>(row.dual_sol());
		*(iter++) = static_cast<value_type>(row.age());
		*(iter++) = static_cast<value_type>(row.lp_activity());
		*(iter++) = static_cast<value_type>(row.obj_cos_sim());
		*(iter++) = static_cast<value_type>(row.l2_norm());
		*(iter++) = static_cast<value_type>(row.is_at_rhs());
		*(iter++) = static_cast<value_type>(row.is_at_lhs());
		*(iter++) = static_cast<value_type>(row.is_local());
		*(iter++) = static_cast<value_type>(row.is_modifiable());
		*(iter++) = static_cast<value_type>(row.is_removable());
		iter[static_cast<std::size_t>(row.basis_status())] = 1.;
		iter += scip::enum_size<scip::base_stat>::value;
		count++;
	}

	// Make sure we iterated over as many element as there are in the tensor
	auto diff = iter - row_feat.begin();
	(void)diff;
	assert(static_cast<std::size_t>(iter - row_feat.begin()) == row_feat.size());

	return row_feat;
}

auto NodeBipartite::get(environment::State const& state) -> NodeBipartiteObs {
	return {extract_col_feat(state.model), extract_row_feat(state.model)};
}

}  // namespace observation
}  // namespace ecole
