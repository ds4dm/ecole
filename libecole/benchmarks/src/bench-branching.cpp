#include <chrono>
#include <tuple>
#include <utility>

#include <scip/scip.h>

#include "ecole/dynamics/branching.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/utility/chrono.hpp"

#include "bench-branching.hpp"
#include "branching/index-branchrule.hpp"

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

}  // namespace

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

auto benchmark_branching(scip::Model model, Tags tags) -> Result {
	return benchmark_lambda(
		{{"branching_rule", &measure_branching_rule}, {"branching_dynamics", &measure_branching_dynamics}},
		std::move(model),
		std::move(tags));
}

auto benchmark_branching(ModelGenerator gen, std::size_t n, Tags tags) -> std::vector<Result> {
	return benchmark_lambda(
		{{"branching_rule", &measure_branching_rule}, {"braching_dynamics", &measure_branching_dynamics}},
		std::move(gen),
		n,
		std::move(tags));
}

}  // namespace ecole::benchmark
