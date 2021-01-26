#pragma once

#include <map>
#include <string>

#include "ecole/scip/model.hpp"

namespace ecole::benchmark {

struct Instance {
	std::string name = "unknown-model";
	std::size_t n_vars = 0;
	std::size_t n_cons = 0;
	std::size_t n_nonzero = 0;

	static auto from_model(scip::Model const& model) -> Instance;
};

struct Metrics {
	double wall_time_s = 0.;
	double cpu_time_s = 0.;
	std::size_t n_nodes = 0;
	std::size_t n_lp_iterations = 0;
};

struct Run {
	Instance instance;
	std::map<std::string, Metrics> metrics;
};

}  // namespace ecole::benchmark
