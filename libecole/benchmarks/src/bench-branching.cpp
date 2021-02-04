#include <chrono>
#include <tuple>
#include <utility>

#include <fmt/format.h>
#include <scip/scip.h>

#include "ecole/dynamics/branching.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

#include "bench-branching.hpp"
#include "branching/index-branchrule.hpp"
#include "csv.hpp"

namespace ecole::benchmark {

namespace {

template <typename Func> auto measure_on_model(Func&& func_to_bench, scip::Model model) -> Metrics {
	auto const cpu_time_before = utility::cpu_clock::now();
	auto const wall_time_before = std::chrono::steady_clock::now();
	func_to_bench(model);
	auto const wall_time_after = std::chrono::steady_clock::now();
	auto const cpu_time_after = utility::cpu_clock::now();

	return {
		std::chrono::duration<double>(wall_time_after - wall_time_before).count(),
		std::chrono::duration<double>(cpu_time_after - cpu_time_before).count(),
		static_cast<std::size_t>(SCIPgetNTotalNodes(model.get_scip_ptr())),
		static_cast<std::size_t>(SCIPgetNLPIterations(model.get_scip_ptr())),
	};
}

auto measure_branching_dynamics(scip::Model model) -> Metrics {
	return measure_on_model(
		[](scip::Model& m) {
			auto dyn = dynamics::BranchingDynamics{};
			auto [done, action_set] = dyn.reset_dynamics(m);
			while (!done) {
				std::tie(done, action_set) = dyn.step_dynamics(m, action_set.value()[0]);
			}
		},
		std::move(model));
}

auto measure_branching_rule(scip::Model model) -> Metrics {
	return measure_on_model(
		[](scip::Model& m) {
			auto* branch_rule = new ecole::scip::IndexBranchrule{m.get_scip_ptr(), "FirstVarBranching", 0UL};
			SCIPincludeObjBranchrule(m.get_scip_ptr(), branch_rule, true);
			// NOLINTNEXTLINE dynamically allocated object ownership is given to SCIP
			m.solve();
		},
		std::move(model));
}

}  // namespace

auto BranchingResult::csv_title() -> std::string {
	return merge_csv(
		InstanceFeatures::csv_title(), Metrics::csv_title("branching_dynamics:"), Metrics::csv_title("branching_rule:"));
}

auto BranchingResult::csv() -> std::string {
	return merge_csv(instance.csv(), branching_dynamics_metrics.csv(), branching_rule_metrics.csv());
}

auto benchmark_branching(scip::Model const& model) -> BranchingResult {
	return {
		InstanceFeatures::from_model(model.copy_orig()),
		measure_branching_dynamics(model.copy_orig()),
		measure_branching_rule(model.copy_orig()),
	};
}

}  // namespace ecole::benchmark
