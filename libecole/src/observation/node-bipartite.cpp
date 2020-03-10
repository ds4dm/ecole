#include <array>
#include <cstddef>
#include <limits>

#include <xtensor/xview.hpp>

#include "ecole/observation/node-bipartite.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace observation {

auto NodeBipartite::clone() const -> std::unique_ptr<Base> {
	return std::make_unique<NodeBipartite>(*this);
}

using tensor = decltype(NodeBipartiteObs::col_feat);
using value_type = tensor::value_type;
constexpr auto nan = std::numeric_limits<value_type>::quiet_NaN();

constexpr std::size_t n_col_feat() {
	return (
		11 + scip::enum_size<scip::var_type>::value +
		scip::enum_size<scip::base_stat>::value);
}

static auto extract_col_feat(scip::Model const& model) {
	tensor col_feat{{model.variables().size, n_col_feat()}, 0.};

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

auto NodeBipartite::get(environment::State const& state) -> NodeBipartiteObs {
	return {extract_col_feat(state.model)};
}

}  // namespace observation
}  // namespace ecole
