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
auto benchmark_lambda(CompetitorMap const& competitors, scip::Model model, Tags tags = {}) -> Result;
auto benchmark_lambda(CompetitorMap const& competitors, ModelGenerator gen, std::size_t n, Tags tags = {})
	-> std::vector<Result>;

}  // namespace ecole::benchmark
