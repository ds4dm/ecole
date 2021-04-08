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
using Tags = std::vector<std::string>;

struct Result {
	InstanceFeatures instance;
	MetricsMap metrics;
	Tags tags;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(Result, instance, metrics, tags);
};

auto run(CompetitorMap const& competitors, scip::Model model, Tags tags) -> Result;
auto run(CompetitorMap const& competitors, scip::Model model) -> Result;
auto run(CompetitorMap const& competitors, std::vector<scip::Model> models, Tags tags) -> std::vector<Result>;
auto run(CompetitorMap const& competitors, std::vector<scip::Model> models) -> std::vector<Result>;

template <typename Generator> auto generate(Generator gen, std::size_t n) -> std::vector<scip::Model>;

/********************
 *  Implementation  *
 ********************/

template <typename Generator> auto generate(Generator gen, std::size_t n) -> std::vector<scip::Model> {
	auto instances = std::vector<scip::Model>{};
	instances.reserve(n);
	for (std::size_t i = 0; i < n; ++i) {
		instances.emplace_back(gen.next());
	}
	return instances;
}

}  // namespace ecole::benchmark
