#pragma once

#include <map>
#include <string>
#include <type_traits>
#include <vector>

#include <nlohmann/json.hpp>

#include "ecole/scip/model.hpp"

namespace ecole::benchmark {

struct Instance {
	std::string name = "unknown-model";
	std::size_t n_vars = 0;
	std::size_t n_cons = 0;
	std::size_t n_nonzero = 0;

	static auto from_model(scip::Model const& model) -> Instance;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Instance, name, n_vars, n_cons, n_nonzero);
};

struct Metrics {
	double wall_time_s = 0.;
	double cpu_time_s = 0.;
	std::size_t n_nodes = 0;
	std::size_t n_lp_iterations = 0;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Metrics, wall_time_s, cpu_time_s, n_nodes, n_lp_iterations);
};

struct Run {
	Instance instance;
	std::map<std::string, Metrics> metrics;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Run, instance, metrics);
};

using Competitor = std::add_pointer_t<Metrics(scip::Model)>;

auto run(std::map<std::string, Competitor> const& competitors, scip::Model model) -> Run;
auto run(std::map<std::string, Competitor> const& competitors, std::vector<scip::Model> models) -> std::vector<Run>;

}  // namespace ecole::benchmark
