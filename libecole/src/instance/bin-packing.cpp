#include <array>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <scip/type_var.h>
#include <string>
#include <utility>
#include <vector>

#include "ecole/instance/bin-packing.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"
#include "ecole/scip/var.hpp"
#include <fmt/format.h>
#include <range/v3/view/enumerate.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xbuilder.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xrandom.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

namespace ecole::instance {

/**************************************************
 *  Binpacking methods  *
 **************************************************/

Binpacking::Binpacking(Binpacking::Parameters parameters_, RandomGenerator rng_) :
	rng{rng_}, parameters{std::move(parameters_)} {}
Binpacking::Binpacking(Binpacking::Parameters parameters_) : Binpacking{parameters_, ecole::spawn_random_generator()} {}
Binpacking::Binpacking() : Binpacking(Parameters{}) {}

scip::Model Binpacking::next() {
	return generate_instance(parameters, rng);
}

void Binpacking::seed(Seed seed) {
	rng.seed(seed);
}

/*************************************************************
 *  Binpacking::generate_instance  *
 *************************************************************/

namespace {

using value_type = SCIP_Real;
using xvector = xt::xtensor<value_type, 1>;
using xmatrix = xt::xtensor<value_type, 2>;

auto read_problem(
	std::string& filename, /**< filename */
	int& n_items,          /**< capacity in instance */
	int& capacity,
	std::vector<double>& weights /**< array of demands of instance */
) {

	SCIP_FILE* file;
	SCIP_Bool error;
	char name[SCIP_MAXSTRLEN];
	char format[16];
	char buffer[SCIP_MAXSTRLEN];
	int bestsolvalue;
	int nread;
	int weight;
	int n_weights;
	int lineno;

	file = SCIPfopen(filename.c_str(), "r");
	/* open file */
	if (file == NULL) {
		std::cerr << fmt::format("cannot open file <{}> for reading\n", filename);
		SCIPprintSysError(filename.c_str());
		return SCIP_NOFILE;
	}

	lineno = 0;
	std::cout << name << "++ uninitialized ++";

	/* read problem name */
	if (!SCIPfeof(file)) {
		/* get next line */
		if (SCIPfgets(buffer, (int)sizeof(buffer), file) == NULL) return SCIP_READERROR;
		lineno++;

		/* parse dimension line */
		sprintf(format, "%%%ds\n", SCIP_MAXSTRLEN);
		nread = sscanf(buffer, format, name);
		if (nread == 0) {
			std::cerr << fmt::format("invalid input line {} in file <{}>: <{}>\n", lineno, filename, buffer);
			return SCIP_READERROR;
		}

		std::cout << fmt::format("problem name <{}>\n", name);
	}

	capacity = 0;
	n_items = 0;

	/* read problem dimension */
	if (!SCIPfeof(file)) {
		/* get next line */
		if (SCIPfgets(buffer, (int)sizeof(buffer), file) == NULL) return SCIP_READERROR;
		lineno++;

		/* parse dimension line */
		nread = sscanf(buffer, "%d %d %d\n", &capacity, &n_items, &bestsolvalue);
		if (nread < 2) {
			std::cerr << fmt::format("invalid input line {} in file <{}>: <{}>\n", lineno, filename, buffer);
			return SCIP_READERROR;
		}

		std::cerr << fmt::format(
			"capacity = <{}>, number of items = <{}>, best known solution = <{}>\n", capacity, n_items, bestsolvalue);
	}

	/* parse weights */
	weights.resize(n_items, 0);
	n_weights = 0;
	error = FALSE;

	while (!SCIPfeof(file) && !error) {
		/* get next line */
		if (SCIPfgets(buffer, (int)sizeof(buffer), file) == NULL) break;
		lineno++;

		/* parse the line */
		nread = sscanf(buffer, "%d\n", &weight);
		if (nread == 0) {
			std::cerr << fmt::format("invalid input line {} in file <{}>: <{}>\n", lineno, filename, buffer);
			error = TRUE;
			break;
		}

		weights[n_weights] = weight;
		n_weights++;

		if (n_weights == n_items) break;
	}

	if (n_weights < n_items) {
		std::cerr << fmt::format(
			"set n_items from <{}> to <{}> since the file <{}> only contains <{}> weights\n",
			n_items,
			n_weights,
			filename,
			n_weights);
		n_items = n_weights;
	}

	(void)SCIPfclose(file);

	if (error) return SCIP_READERROR;
	return SCIP_OKAY;
}

/** Create and add a single continuous variable the for the fraction of item weight (customer demand) served by the bin
 * (vehicle).
 *
 * Variables are automatically released (using the unique_ptr provided by scip::create_var_basic) after being captured
 * by the scip*. Their lifetime should not exceed that of the scip* (although that was already implied when creating
 * them).
 */
auto add_items_var(SCIP* scip, std::size_t i, std::size_t j, SCIP_Real cost, bool continuous) -> SCIP_VAR* {
	auto const name = fmt::format("x_{}_{}", i, j);
	auto unique_var = scip::create_var_basic(
		scip, name.c_str(), 0.0, 1.0, 0.0, /*add options for continuous variables */ SCIP_VARTYPE_BINARY);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

/** Create and add all variables for accumulated_weights the fraction of items weights (customer demands) from bins
 * (vehicles).
 *
 * Variables pointers are returned in a symmetric n_customers matrix .
 */
auto add_bins_items_vars(SCIP* scip, int n_bins, int n_items, xvector const& weights, bool continuous) {
	// symmetric matrix
	assert(weights.size() == n_items);

	auto vars = xt::xtensor<SCIP_VAR*, 2>{{n_bins, n_items}, nullptr};
	for (std::size_t i = 0; i < n_bins; ++i) {
		for (std::size_t j = 0; j < n_items; ++j) {
			vars(i, j) = add_items_var(scip, i, j, weights[j], continuous);
		}
	}
	return vars;
}

/** Create and add a single integer variable the representing the assignment of the item.
 *
 * Variables are automatically released (using the unique_ptr provided by scip::create_var_basic) after being captured
 * by the scip*. Their lifetime should not exceed that of the scip* (although that was already implied when creating
 * them).
 */
auto add_bins_var(SCIP* scip, std::size_t idx, double bin_cost) -> SCIP_VAR* {
	auto const name = fmt::format("y_{}", idx);
	auto unique_var = scip::create_var_basic(scip, name.c_str(), 0., 1., bin_cost, SCIP_VARTYPE_BINARY);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

auto add_bins_vars(SCIP* scip, std::size_t n_bins, double bin_cost) {
	auto vars = xt::xtensor<SCIP_VAR*, 1>({n_bins}, nullptr);
	auto* out_iter = vars.begin();
	for (std::size_t i = 0; i < n_bins; ++i) {
		*(out_iter++) = add_bins_var(scip, i, bin_cost);
	}
	return vars;
}

/* capacity constraints */
auto add_capacity_cons(
	SCIP* scip,
	xt::xtensor<SCIP_VAR*, 2> const& bins_items_vars,
	xt::xtensor<SCIP_VAR*, 1> const& bins_vars,
	xvector const& weights,
	int capacity) -> void {

	auto const inf = SCIPinfinity(scip);

	auto const [n_bins, n_items] = bins_items_vars.shape();

	assert(weights.size() == n_items);

	std::vector<value_type> coefs(weights.begin(), weights.end());
	coefs.push_back(-(SCIP_Real)capacity);

	assert(coefs.size() == n_items + 1);

	std::vector<std::size_t> shape = {n_items + 1};
	for (std::size_t i = 0; i < n_bins; ++i) {
		auto const name = fmt::format("c_{}", i);
		auto bins_items_vars_row = xt::row(bins_items_vars, i);
		std::vector<SCIP_VAR*> vars(bins_items_vars_row.begin(), bins_items_vars_row.end());
		vars.push_back(bins_vars(i));
		auto cons = scip::create_cons_basic_linear(scip, name.c_str(), vars.size(), vars.data(), coefs.data(), -inf, 0.);
		scip::call(SCIPaddCons, scip, cons.get());
	}
}

// ensures that each item is packed only once (tightening)
auto add_tightening_cons(SCIP* scip, xt::xtensor<SCIP_VAR*, 2> bins_items_vars) -> void {
	auto const inf = SCIPinfinity(scip);

	auto const [n_bins, n_items] = bins_items_vars.shape();
	for (std::size_t i = 0; i < n_items; ++i) {
		auto name = fmt::format("tightening_cons_item_{}", i);
		auto const coefs = xvector({n_bins}, 1.);
		auto row = xt::col(bins_items_vars, i);
		std::vector<SCIP_VAR*> vars(row.begin(), row.end());
		auto cons = scip::create_cons_basic_linear(scip, name.c_str(), n_bins, vars.data(), coefs.data(), 1.0, 1.0);
		scip::call(SCIPaddCons, scip, cons.get());
	}
}

}  // namespace

scip::Model Binpacking::generate_instance(Binpacking::Parameters parameters, RandomGenerator& rng) {

	double bin_cost = 1.0;               // NOLINT(readability-magic-numbers)
	int capacity;                        // NOLINT(readability-magic-numbers)
	int n_items;                         // NOLINT(readability-magic-numbers)
	bool continuous_assignment = false;  // NOLINT(readability-magic-numbers)
	std::vector<double> weights;         // NOLINT(readability-magic-numbers)

	if (!read_problem(parameters.filename, n_items, capacity, weights)) {
		throw SCIP_READERROR;
	}

	auto xweights = static_cast<xvector>(xt::adapt(weights));

	auto model = scip::Model::prob_basic();
	model.set_name(fmt::format("Binpacking-{}-{}", parameters.n_bins, n_items));

	auto* const scip = model.get_scip_ptr();

	auto const bins_vars = add_bins_vars(scip, parameters.n_bins, bin_cost);
	auto const bins_items_vars = add_bins_items_vars(scip, parameters.n_bins, n_items, xweights, continuous_assignment);

	add_capacity_cons(scip, bins_items_vars, bins_vars, xweights, capacity);
	add_tightening_cons(scip, bins_items_vars);

	return model;
}

}  // namespace ecole::instance
