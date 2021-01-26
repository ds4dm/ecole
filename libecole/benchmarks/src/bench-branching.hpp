#pragma once

#include "ecole/scip/model.hpp"

#include "benchmark.hpp"

namespace ecole::benchmark {

auto benchmark_branching_dynamics(scip::Model model) -> Metrics;

auto benchmark_branching_rule(scip::Model model) -> Metrics;

}  // namespace ecole::benchmark
