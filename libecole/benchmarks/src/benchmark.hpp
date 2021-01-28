#pragma once

#include <map>
#include <string>
#include <type_traits>
#include <vector>

#include <nlohmann/json.hpp>

#include "ecole/scip/model.hpp"

namespace ecole::benchmark {

struct InstanceFeatures {
	std::size_t n_vars = 0;
	std::size_t n_cons = 0;
	std::size_t n_nonzero = 0;

	static auto from_model(scip::Model const& model) -> InstanceFeatures;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(InstanceFeatures, n_vars, n_cons, n_nonzero);
};

struct Metrics {
	double wall_time_s = 0.;
	double cpu_time_s = 0.;
	std::size_t n_nodes = 0;
	std::size_t n_lp_iterations = 0;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Metrics, wall_time_s, cpu_time_s, n_nodes, n_lp_iterations);
};

using CompetirorId = std::string;
using Competitor = std::add_pointer_t<Metrics(scip::Model)>;
using CompetitorMap = std::map<CompetirorId, Competitor>;
using MetricsMap = std::map<CompetirorId, Metrics>;

struct Result {
	InstanceFeatures instance;
	MetricsMap metrics;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Result, instance, metrics);
};

auto run(CompetitorMap const& competitors, scip::Model model) -> Result;
auto run(CompetitorMap const& competitors, std::vector<scip::Model> models) -> std::vector<Result>;

}  // namespace ecole::benchmark
