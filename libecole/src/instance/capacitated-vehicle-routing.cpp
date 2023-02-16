#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <scip/type_var.h>
#include <string>
#include <utility>
#include <vector>

#include "ecole/instance/capacitated-vehicle-routing.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"
#include "ecole/scip/var.hpp"
#include <fmt/format.h>
#include <range/v3/view/enumerate.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xrandom.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

namespace views = ranges::views;

namespace ecole::instance {

/**************************************************
 *  CapacitatedVehicleRoutingLoader methods  *
 **************************************************/

CapacitatedVehicleRoutingLoader::CapacitatedVehicleRoutingLoader(
	CapacitatedVehicleRoutingLoader::Parameters parameters_,
	RandomGenerator rng_) :
	rng{rng_}, parameters{std::move(parameters_)} {}
CapacitatedVehicleRoutingLoader::CapacitatedVehicleRoutingLoader(
	CapacitatedVehicleRoutingLoader::Parameters parameters_) :
	CapacitatedVehicleRoutingLoader{parameters_, ecole::spawn_random_generator()} {}
CapacitatedVehicleRoutingLoader::CapacitatedVehicleRoutingLoader() : CapacitatedVehicleRoutingLoader(Parameters{}) {}

scip::Model CapacitatedVehicleRoutingLoader::next() {
	return generate_instance(parameters, rng);
}

void CapacitatedVehicleRoutingLoader::seed(Seed seed) {
	rng.seed(seed);
}

/*************************************************************
 *  CapacitatedVehicleRoutingLoader::generate_instance  *
 *************************************************************/

namespace {

using value_type = SCIP_Real;
using xvector = xt::xtensor<value_type, 1>;
using xmatrix = xt::xtensor<value_type, 2>;

auto read_problem(
	std::string& filename,                    /**< filename */
	std::size_t& n_customers,                 /**< number of nodes in instance */
	int& capacity,                            /**< capacity in instance */
	std::vector<int>& demand,                 /**< array of demands of instance */
	std::vector<std::vector<SCIP_Real>>& dist /**< distances between nodes*/
) {
	static const std::string DIMENSION = "DIMENSION";
	static const std::string DEMAND_SECTION = "DEMAND_SECTION";
	static const std::string DEPOT_SECTION = "DEPOT_SECTION";
	static const std::string EDGE_WEIGHT_TYPE = "EDGE_WEIGHT_TYPE";
	static const std::string EUC_2D = "EUC_2D";
	static const std::string EXPLICIT = "EXPLICIT";
	static const std::string LOWER_DIAG_ROW = "LOWER_DIAG_ROW";
	static const std::string EDGE_WEIGHT_FORMAT = "EDGE_WEIGHT_FORMAT";
	static const std::string EDGE_WEIGHT_SECTION = "EDGE_WEIGHT_SECTION";
	static const std::string NODE_COORD_SECTION = "NODE_COORD_SECTION";
	static const std::string CAPACITY = "CAPACITY";

	std::ifstream file(filename);

	if (!file) {
		std::cerr << "Cannot open file " << filename << std::endl;
		return 1;
	}

	std::string edge_weight_type = "";
	std::string edge_weight_format = "";
	std::vector<int> x;
	std::vector<int> y;

	while (file) {
		//--------------------
		// Read keyword.
		//--------------------
		std::string key;
		std::string dummy;
		file >> key;

		if (key == DIMENSION) {
			file >> dummy;
			file >> n_customers;
			demand.resize(n_customers, 0); /*lint !e732 !e747*/
			dist.resize(n_customers);      /*lint !e732 !e747*/
			for (int i = 0; i < n_customers; ++i)
				dist[i].resize(n_customers, 0); /*lint !e732 !e747*/
		}

		if (key == CAPACITY) {
			file >> dummy;
			file >> capacity;
		} else if (key == EDGE_WEIGHT_TYPE) {
			file >> dummy;
			file >> edge_weight_type;
			if (edge_weight_type != EUC_2D && edge_weight_type != EXPLICIT) {
				std::cerr << "Wrong " << EDGE_WEIGHT_TYPE << " " << edge_weight_type << std::endl;
				return 1;
			}
			if (edge_weight_type == EUC_2D) {
				x.resize(n_customers, 0); /*lint !e732 !e747*/
				y.resize(n_customers, 0); /*lint !e732 !e747*/
			}
		} else if (key == EDGE_WEIGHT_FORMAT) {
			file >> dummy;
			file >> edge_weight_format;
		} else if (key == EDGE_WEIGHT_FORMAT + ":") {
			file >> edge_weight_format;
		} else if (key == EDGE_WEIGHT_SECTION) {
			if (edge_weight_type != EXPLICIT || edge_weight_format != LOWER_DIAG_ROW) {
				std::cerr << "Error. Unsupported edge length type." << std::endl;
				return 1;
			}
			for (int i = 0; i < n_customers; ++i) {
				for (int j = 0; j < n_customers; ++j) {
					int l;
					file >> l;
					dist[i][j] = l; /*lint !e732 !e747*/
				}
			}
		} else if (key == NODE_COORD_SECTION) {
			if (edge_weight_type != EUC_2D) {
				std::cerr << "Error. Data file contains " << EDGE_WEIGHT_TYPE << " " << edge_weight_type << " and "
									<< NODE_COORD_SECTION << std::endl;
				return 1;
			}
			for (int i = 0; i < n_customers; ++i) {
				int j, xi, yi;
				file >> j;
				file >> xi;
				file >> yi;
				if (j != i + 1) {
					std::cerr << "Error reading " << NODE_COORD_SECTION << std::endl;
					return 1;
				}
				x[i] = xi; /*lint !e732 !e747*/
				y[i] = yi; /*lint !e732 !e747*/
			}
			for (int i = 0; i < n_customers; ++i) {
				for (int j = 0; j < n_customers; ++j) {
					int dx = x[i] - x[j];                            /*lint !e732 !e747 !e864*/
					int dy = y[i] - y[j];                            /*lint !e732 !e747 !e864*/
					dist[i][j] = (SCIP_Real)sqrt(dx * dx + dy * dy); /*lint !e732 !e747 !e790*/
				}
			}
		} else if (key == DEMAND_SECTION) {
			for (int i = 0; i < n_customers; ++i) {
				int j, d;
				file >> j;
				file >> d;
				if (j != i + 1) {
					std::cerr << "Error reading " << DEMAND_SECTION << std::endl;
					return 1;
				}
				demand[i] = d; /*lint !e732 !e747*/
			}
		} else if (key == DEPOT_SECTION) {
			for (int i = 0; i != -1;) {
				file >> i;
				if (i != -1 && i != 1) {
					std::cerr << "Error: This file specifies other depots than 1." << std::endl;
					return 1;
				}
			}
		} else {
			(void)getline(file, dummy);
		}
	}

	return 0;
}

/** Create and add a single binary variable the for the fraction of customer demand served by the vehicle.
 *
 * Variables are automatically released (using the unique_ptr provided by scip::create_var_basic) after being captured
 * by the scip*. Their lifetime should not exceed that of the scip* (although that was already implied when creating
 * them).
 */
auto add_serving_var(SCIP* scip, std::size_t i, std::size_t j, SCIP_Real cost, bool continuous) -> SCIP_VAR* {
	auto const name = fmt::format("x_{}_{}", i, j);
	auto unique_var = scip::create_var_basic(
		scip, name.c_str(), 0.0, 1.0, cost, /*add options for continuous variables */ SCIP_VARTYPE_BINARY);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

/** Create and add all variables for serving the fraction of customer demands from vehicles.
 *
 * Variables pointers are returned in a symmetric n_customers matrix .
 */
auto add_serving_vars(SCIP* scip, std::vector<std::vector<SCIP_Real>>& transportation_costs, bool continuous) {
	// symmetric matrix
	auto const n_customers = transportation_costs.size();
	auto vars = xt::xtensor<SCIP_VAR*, 2>{{n_customers, n_customers}, nullptr};
	for (std::size_t i = 0; i < n_customers; ++i) {
		for (std::size_t j = 0; j < n_customers; ++j) {
			if (i != j) {
				auto cost = transportation_costs[i][j];
				vars(i, j) = add_serving_var(scip, i, j, cost, continuous);
			}
		}
	}
	return vars;
}

/** Create and add a single integer variable the representing the assignment of the vehicle.
 *
 * Variables are automatically released (using the unique_ptr provided by scip::create_var_basic) after being captured
 * by the scip*. Their lifetime should not exceed that of the scip* (although that was already implied when creating
 * them).
 */
auto add_accumulated_demand_var(SCIP* scip, std::size_t idx, int capacity) -> SCIP_VAR* {
	auto const name = fmt::format("u_{}", idx);
	auto unique_var = scip::create_var_basic(scip, name.c_str(), 0.0, capacity, 0.0, SCIP_VARTYPE_CONTINUOUS);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

auto add_accumulated_demand_vars(SCIP* scip, std::size_t n_customers, int capacity) {
	auto vars = xt::xtensor<SCIP_VAR*, 1>({n_customers}, nullptr);
	auto* out_iter = vars.begin();
	for (std::size_t i = 1; i < n_customers; ++i) {
		/*pre-incrementing out_iter to start assigning from 1*/
		*(++out_iter) = add_accumulated_demand_var(scip, i, capacity);
	}
	return vars;
}

/* capacity constraints */
auto add_capacity_cons(
	SCIP* scip,
	xt::xtensor<SCIP_VAR*, 1> const& accumulated_demand_vars,
	xvector const& demands,
	int capacity) -> void {

	auto const [n_customers] = accumulated_demand_vars.shape();
	assert(demands.size() == n_customers);

	for (std::size_t i = 1; i < n_customers; ++i) {
		auto const name = fmt::format("c_{}", i);
		auto constexpr coefs = std::array<SCIP_Real, 1>{1.};
		auto cons = scip::create_cons_basic_linear(
			scip, name.c_str(), 1, &accumulated_demand_vars[i], coefs.data(), demands[i], capacity);
		scip::call(SCIPaddCons, scip, cons.get());
	}
}

// Miller-Tucker-Zemlin SEC constraints
auto add_mtz_cons(
	SCIP* scip,
	xt::xtensor<SCIP_VAR*, 2> const& serving_vars,
	xt::xtensor<SCIP_VAR*, 1> const& accumulated_demand_vars,
	xvector const& demands,
	int capacity) -> void {

	auto const inf = SCIPinfinity(scip);
	auto const [n_customers, THROW_AWAY] = serving_vars.shape();
	assert(accumulated_demand_vars.size() == n_customers);

	for (std::size_t i = 1; i < n_customers; ++i) {
		for (std::size_t j = 1; j < n_customers; ++j) {
			if (i != j) {
				auto const mtz_se_con_name = fmt::format("mtz_se_con_{}_{}", i, j);
				// u[i] - u[j]
				auto coefs = std::array<SCIP_Real, 3>{1.0, -1.0, (SCIP_Real)capacity};
				auto vars =
					std::array<SCIP_VAR*, 3>{accumulated_demand_vars[i], accumulated_demand_vars[j], serving_vars(i, j)};
				auto cons = scip::create_cons_basic_linear(
					scip, mtz_se_con_name.c_str(), vars.size(), vars.data(), coefs.data(), -inf, capacity - demands[j]);
				scip::call(SCIPaddCons, scip, cons.get());
			}
		}
	}
}

/* add arc-routing - degree constraints */
auto add_degree_out_cons(SCIP* scip, xt::xtensor<SCIP_VAR*, 2> const& serving_vars, std::size_t n_vehicles) -> void {
	auto const [n_customers, THROW_AWAY] = serving_vars.shape();
	for (std::size_t j = 0; j < n_customers; ++j) {
		auto const name = fmt::format("deg_con_out_{}", j);
		auto cons = scip::create_cons_basic_linear(
			scip,
			name.c_str(),
			0,
			nullptr,
			nullptr,
			j > 0 ? 1.0 : n_vehicles,  /* lhs */
			j > 0 ? 1.0 : n_vehicles); /* rhs */
		for (std::size_t i = 0; i < n_customers; ++i) {
			if (i != j) {
				scip::call(SCIPaddCoefLinear, scip, cons.get(), serving_vars(i, j), 1.0);
			}
		}
		scip::call(SCIPaddCons, scip, cons.get());
	}
}

auto add_degree_in_cons(SCIP* scip, xt::xtensor<SCIP_VAR*, 2> const& serving_vars, std::size_t n_vehicles) -> void {
	auto const [n_customers, THROW_AWAY] = serving_vars.shape();
	for (std::size_t i = 0; i < n_customers; ++i) {
		auto const name = fmt::format("deg_con_in_{}", i);
		auto cons = scip::create_cons_basic_linear(
			scip,
			name.c_str(),
			0,
			nullptr,
			nullptr,
			i > 0 ? 1.0 : n_vehicles,  /* lhs */
			i > 0 ? 1.0 : n_vehicles); /* rhs */

		for (std::size_t j = 0; j < n_customers; ++j) {
			if (j != i) {
				scip::call(SCIPaddCoefLinear, scip, cons.get(), serving_vars(i, j), 1.0);
			}
		}
		scip::call(SCIPaddCons, scip, cons.get());
	}
}

}  // namespace

scip::Model CapacitatedVehicleRoutingLoader::generate_instance(
	CapacitatedVehicleRoutingLoader::Parameters parameters,
	RandomGenerator& rng /*not used*/) {

	std::size_t n_customers;                   // NOLINT(readability-magic-numbers)
	int capacity;                              // NOLINT(readability-magic-numbers)
	bool continuous_assignment = false;        // NOLINT(readability-magic-numbers)
	std::vector<int> demands_;                 // NOLINT(readability-magic-numbers)
	std::vector<std::vector<SCIP_Real>> dist;  // NOLINT(readability-magic-numbers)

	if (read_problem(parameters.filename, n_customers, capacity, demands_, dist)) {
		throw SCIP_READERROR;
	}

	// // Customer demand
	auto const demands = static_cast<xvector>(xt::adapt(demands_));

	auto model = scip::Model::prob_basic();
	model.set_name(fmt::format("CapacitatedVehicleRouting-{}-{}", n_customers, parameters.n_vehicles));

	auto* const scip = model.get_scip_ptr();

	auto const serving_vars = add_serving_vars(scip, dist, continuous_assignment);
	auto const accumulated_demand_vars = add_accumulated_demand_vars(scip, n_customers, capacity);

	add_capacity_cons(scip, accumulated_demand_vars, demands, capacity);
	add_mtz_cons(scip, serving_vars, accumulated_demand_vars, demands, capacity);
	add_degree_out_cons(scip, serving_vars, parameters.n_vehicles);
	add_degree_in_cons(scip, serving_vars, parameters.n_vehicles);

	return model;
}

}  // namespace ecole::instance
