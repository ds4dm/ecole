#pragma once

#include <string>

#include "ecole/scip/model.hpp"

#include "benchmark.hpp"

namespace ecole::benchmark {

struct BranchingResult {
	InstanceFeatures instance;
	Metrics branching_dynamics_metrics;
	Metrics branching_rule_metrics;

	static auto csv_title() -> std::string;
	auto csv() -> std::string;
};

/** Benchmark the branching dynamics against a branch rule on a given model. */
auto benchmark_branching(scip::Model const& model) -> BranchingResult;

}  // namespace ecole::benchmark
