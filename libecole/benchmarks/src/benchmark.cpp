#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

#include "ecole/scip/model.hpp"

#include "benchmark.hpp"

namespace ecole::benchmark {

auto InstanceFeatures::from_model(scip::Model const& model) -> InstanceFeatures {
	return {model.variables().size(), model.constraints().size(), model.nnz()};
}

auto benchmark_lambda(CompetitorMap const& competitors, scip::Model model, Tags tags) -> Result {
	auto result = Result{InstanceFeatures::from_model(model), {}, std::move(tags)};
	for (auto const& [name, competitor] : competitors) {
		result.metrics[name] = std::invoke(competitor, model.copy_orig());
	}
	return result;
}

auto benchmark_lambda(CompetitorMap const& competitors, std::vector<scip::Model> models, Tags tags)
	-> std::vector<Result> {
	auto results = std::vector<Result>{};
	results.reserve(models.size());
	std::transform(
		std::make_move_iterator(models.begin()),
		std::make_move_iterator(models.end()),
		std::back_inserter(results),
		[&](scip::Model&& model) { return benchmark_lambda(competitors, std::move(model), tags); });
	return results;
}

}  // namespace ecole::benchmark
