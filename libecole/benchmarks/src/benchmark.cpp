#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

#include <fmt/format.h>

#include "ecole/dynamics/branching.hpp"
#include "ecole/scip/model.hpp"

#include "benchmark.hpp"
#include "csv.hpp"

namespace ecole::benchmark {

auto InstanceFeatures::from_model(scip::Model model) -> InstanceFeatures {
	// Get model to the root note to extract root node info
	auto dyn = dynamics::BranchingDynamics{};
	dyn.reset_dynamics(model);
	// FIXME in practice there is might be LP even if we never branch. Should use SCIP_EVENTTYPE_FIRSTLPSOLVED
	if (model.get_stage() != SCIP_STAGE_SOLVING) {
		return {
			model.variables().size(),
			model.constraints().size(),
		};
	}
	return {
		model.variables().size(),
		model.constraints().size(),
		model.nnz(),
		model.lp_columns().size(),
		model.lp_rows().size(),
		model.name()};
}

auto InstanceFeatures::csv_title() -> std::string {
	return make_csv("n_vars", "n_cons", "root_nnz", "root_n_cols", "root_n_rows", "name");
}

auto InstanceFeatures::csv() -> std::string {
	return make_csv(n_vars, n_cons, root_nnz, root_n_cols, root_n_rows, name);
}

auto Metrics::csv_title(std::string_view prefix) -> std::string {
	return make_csv(
		fmt::format("{}{}", prefix, "wall_time_s"),
		fmt::format("{}{}", prefix, "cpu_time_s"),
		fmt::format("{}{}", prefix, "n_nodes"),
		fmt::format("{}{}", prefix, "n_lp_iterations"));
}

auto Metrics::csv() -> std::string {
	return make_csv(wall_time_s, cpu_time_s, n_nodes, n_lp_iterations);
}

}  // namespace ecole::benchmark
