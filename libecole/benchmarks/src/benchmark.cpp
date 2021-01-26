#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

#include "ecole/scip/model.hpp"

#include "benchmark.hpp"

namespace ecole::benchmark {

auto Instance::from_model(scip::Model const& model) -> Instance {
	return {model.name(), model.variables().size(), model.constraints().size(), model.nnz()};
}

auto run(std::map<std::string, Competitor> const& competitors, scip::Model model) -> Run {
	auto run = Run{Instance::from_model(model), {}};
	for (auto const& [name, competitor] : competitors) {
		run.metrics[name] = std::invoke(competitor, model.copy_orig());
	}
	return run;
}

auto run(std::map<std::string, Competitor> const& competitors, std::vector<scip::Model> models) -> std::vector<Run> {
	auto runs = std::vector<Run>{};
	runs.reserve(models.size());
	std::transform(
		std::make_move_iterator(models.begin()),
		std::make_move_iterator(models.end()),
		std::back_inserter(runs),
		[&competitors](scip::Model&& model) { return run(competitors, std::move(model)); });
	return runs;
}

}  // namespace ecole::benchmark
