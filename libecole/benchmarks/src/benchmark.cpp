#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

#include "ecole/dynamics/branching.hpp"
#include "ecole/scip/model.hpp"

#include "benchmark.hpp"

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
		model.lp_rows().size()};
}

auto benchmark_lambda(CompetitorMap const& competitors, scip::Model model, Tags tags) -> Result {
	auto metrics = MetricsMap{};
	for (auto const& [name, competitor] : competitors) {
		metrics[name] = std::invoke(competitor, model.copy_orig());
	}
	return {InstanceFeatures::from_model(std::move(model)), std::move(metrics), std::move(tags)};
}

auto benchmark_lambda(CompetitorMap const& competitors, ModelGenerator gen, std::size_t n, Tags tags)
	-> std::vector<Result> {
	auto results = std::vector<Result>{};
	results.reserve(n);
	std::generate_n(std::back_inserter(results), n, [&]() { return benchmark_lambda(competitors, gen(), tags); });
	return results;
}

}  // namespace ecole::benchmark
