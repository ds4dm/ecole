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

auto run(CompetitorMap const& competitors, scip::Model model) -> Result {
	auto result = Result{InstanceFeatures::from_model(model), {}};
	for (auto const& [name, competitor] : competitors) {
		result.metrics[name] = std::invoke(competitor, model.copy_orig());
	}
	return result;
}

auto run(CompetitorMap const& competitors, std::vector<scip::Model> models) -> std::vector<Result> {
	auto results = std::vector<Result>{};
	results.reserve(models.size());
	std::transform(
		std::make_move_iterator(models.begin()),
		std::make_move_iterator(models.end()),
		std::back_inserter(results),
		[&competitors](scip::Model&& model) { return run(competitors, std::move(model)); });
	return results;
}

}  // namespace ecole::benchmark
