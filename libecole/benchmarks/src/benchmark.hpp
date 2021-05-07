#pragma once

#include <string>
#include <string_view>

#include "ecole/scip/model.hpp"

namespace ecole::benchmark {

struct InstanceFeatures {
	std::size_t n_vars = 0;
	std::size_t n_cons = 0;
	std::size_t root_nnz = 0;
	std::size_t root_n_cols = 0;
	std::size_t root_n_rows = 0;
	std::string name = {};

	static auto from_model(scip::Model model) -> InstanceFeatures;

	static auto csv_title() -> std::string;
	auto csv() -> std::string;
};

struct Metrics {
	double wall_time_s = 0.;
	double cpu_time_s = 0.;
	std::size_t n_nodes = 0;
	std::size_t n_lp_iterations = 0;

	static auto csv_title(std::string_view prefix = "") -> std::string;
	auto csv() -> std::string;
};

}  // namespace ecole::benchmark
