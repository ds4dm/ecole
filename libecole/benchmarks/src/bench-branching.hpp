#pragma once

#include "ecole/scip/model.hpp"

#include "benchmark.hpp"

namespace ecole::benchmark {

/**
 * Benchmark competitors for branchings.
 */
auto measure_branching_dynamics(scip::Model model) -> Metrics;
auto measure_branching_rule(scip::Model model) -> Metrics;

/**
 * Alias benchmark functions with branching competitors set.
 */
auto benchmark_branching(scip::Model model, Tags tags = {}) -> Result;
auto benchmark_branching(ModelGenerator gen, std::size_t n, Tags tags = {}) -> std::vector<Result>;

}  // namespace ecole::benchmark
