#pragma once

#include <functional>
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
	std::size_t root_nnz = 0;
	std::size_t root_n_cols = 0;
	std::size_t root_n_rows = 0;

	static auto from_model(scip::Model model) -> InstanceFeatures;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(InstanceFeatures, n_vars, n_cons, root_nnz, root_n_cols, root_n_rows);
};

struct Metrics {
	double wall_time_s = 0.;
	double cpu_time_s = 0.;
	std::size_t n_nodes = 0;
	std::size_t n_lp_iterations = 0;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Metrics, wall_time_s, cpu_time_s, n_nodes, n_lp_iterations);
};

using CompetirorId = std::string;
using Competitor = std::function<Metrics(scip::Model)>;
using CompetitorMap = std::map<CompetirorId, Competitor>;
using MetricsMap = std::map<CompetirorId, Metrics>;
using Tags = std::vector<std::string>;

struct Result {
	InstanceFeatures instance;
	MetricsMap metrics;
	Tags tags;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Result, instance, metrics, tags);
};

template <typename Generator> auto generate(Generator gen, std::size_t n) -> std::vector<scip::Model>;

using ModelGenerator = std::function<scip::Model()>;

/**
 * Run the competitors functions on the model, collect intance feature, and set tags.
 */
auto benchmark_lambda(CompetitorMap const& competitors, scip::Model model, Tags tags = {}) -> Result;

/**
 * Run the competitors functions on n generated models, collect intance feature, and set tags.
 */
auto benchmark_lambda(CompetitorMap const& competitors, ModelGenerator gen, std::size_t n, Tags tags = {})
	-> std::vector<Result>;

}  // namespace ecole::benchmark
