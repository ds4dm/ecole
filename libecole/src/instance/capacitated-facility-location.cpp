#include <array>
#include <memory>

#include <fmt/format.h>
#include <range/v3/view/enumerate.hpp>
#include <xtensor/xmath.hpp>
#include <xtensor/xrandom.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/instance/capacitated-facility-location.hpp"
#include "ecole/scip/cons.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"
#include "ecole/scip/var.hpp"

namespace views = ranges::views;

namespace ecole::instance {

/**************************************************
 *  CapacitatedFacilityLocationGenerator methods  *
 **************************************************/

CapacitatedFacilityLocationGenerator::CapacitatedFacilityLocationGenerator(
	RandomEngine random_engine_,
	CapacitatedFacilityLocationGenerator::Parameters parameters_) :
	random_engine{random_engine_}, parameters{parameters_} {}
CapacitatedFacilityLocationGenerator::CapacitatedFacilityLocationGenerator(
	CapacitatedFacilityLocationGenerator::Parameters parameters_) :
	CapacitatedFacilityLocationGenerator{ecole::spawn_random_engine(), parameters_} {}
CapacitatedFacilityLocationGenerator::CapacitatedFacilityLocationGenerator() :
	CapacitatedFacilityLocationGenerator(Parameters{}) {}

scip::Model CapacitatedFacilityLocationGenerator::next() {
	return generate_instance(random_engine, parameters);
}

void CapacitatedFacilityLocationGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

/*************************************************************
 *  CapacitatedFacilityLocationGenerator::generate_instance  *
 *************************************************************/

namespace {

using value_type = scip::real;
using xvector = xt::xtensor<value_type, 1>;
using xmatrix = xt::xtensor<value_type, 2>;

/** Function to sample the unit transporation costs matrix between customers and facilities.
 *
 * The costs are sampled per unit of demand, as described in Cornuejols et al. (1991).
 */
auto unit_transportation_costs(std::size_t n_customers, std::size_t n_facilities, RandomEngine& random_engine)
	-> xmatrix {
	// To sample a random matrix. Explicit casting in return type to avoid xtensor lazy numer generation.
	auto rand = [&random_engine](auto n, auto m) -> xmatrix {
		return xt::random::rand<value_type>({n, m}, 0., 1., random_engine);
	};

	auto constexpr scaling = value_type{10.};
	auto costs = scaling * xt::sqrt(
													 xt::square(rand(n_customers, std::size_t{1}) - rand(std::size_t{1}, n_facilities)) +
													 xt::square(rand(n_customers, std::size_t{1}) - rand(std::size_t{1}, n_facilities)));

	assert(costs.shape()[0] == n_customers);
	assert(costs.shape()[1] == n_facilities);
	return costs;
}

/** Create and add a single binary variable the representing whether to open the facility.
 *
 * Variable are automatically released (using the unique_ptr provided by scip::create_var_basic) after being captured by
 * the scip*.
 * Their lifetime should not exceed that of the scip* (although that was already implied when creating them).
 */
auto add_facility_var(SCIP* scip, std::size_t idx, scip::real cost) -> SCIP_VAR* {
	auto const name = fmt::format("f_{}", idx);
	auto unique_var = scip::create_var_basic(scip, name.c_str(), 0., 1., cost, SCIP_VARTYPE_BINARY);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

/** Create and add all variables for opening the facilities.
 *
 * Variables pointers are returned in a array with as many entries as there are facilities.
 */
auto add_facility_vars(SCIP* scip, xvector const& fixed_costs) {
	auto vars = xt::xtensor<SCIP_VAR*, 1>{fixed_costs.shape(), nullptr};
	auto* out_iter = vars.begin();
	for (auto [idx, cost] : views::enumerate(fixed_costs)) {
		*(out_iter++) = add_facility_var(scip, idx, cost);
	}
	return vars;
}

/** Create and add a single continuous variable the for the fraction of customer demand served by the facility.
 *
 * Variable are automatically released (using the unique_ptr provided by scip::create_var_basic) after being captured by
 * the scip*.
 * Their lifetime should not exceed that of the scip* (although that was already implied when creating them).
 */
auto add_serving_var(SCIP* scip, std::size_t customer_idx, std::size_t facility_idx, scip::real cost) -> SCIP_VAR* {
	auto const name = fmt::format("s_{}_{}", customer_idx, facility_idx);
	auto unique_var = scip::create_var_basic(scip, name.c_str(), 0., 1., cost, SCIP_VARTYPE_CONTINUOUS);
	auto* var_ptr = unique_var.get();
	scip::call(SCIPaddVar, scip, var_ptr);
	return var_ptr;
}

/** Create and add all variables for serving the fraction of customer demands from facilities.
 *
 * Variables pointers are returned in a matrix where rows reprensent customers and columns reprensent facilities.
 */
auto add_serving_vars(SCIP* scip, xmatrix const& transportation_costs) {
	auto const [n_customers, n_facilities] = transportation_costs.shape();
	auto vars = xt::xtensor<SCIP_VAR*, 2>{{n_customers, n_facilities}, nullptr};
	for (std::size_t customer_idx = 0; customer_idx < n_customers; ++customer_idx) {
		for (std::size_t facility_idx = 0; facility_idx < n_facilities; ++facility_idx) {
			auto cost = transportation_costs(customer_idx, facility_idx);
			vars(customer_idx, facility_idx) = add_serving_var(scip, customer_idx, facility_idx, cost);
		}
	}
	return vars;
}

/** Add n_customers constraints for meeting customer demands.
 *
 * For every customer add a constraint that their demand is met through all facilities.
 * That is, fractions served through each facilities sum to one.
 * Constraints are relased automatically (through unique_ptr in scip::create_cons_basic_linear).
 */
auto add_demand_cons(SCIP* scip, xt::xtensor<SCIP_VAR*, 2> const& serving_vars) -> void {
	auto const inf = SCIPinfinity(scip);
	auto const [n_customers, n_facilities] = serving_vars.shape();
	// Asserting row major as we pass the pointer as an array to SCIP when creating constraints
	assert(serving_vars.layout() == xt::layout_type::row_major);

	// Note change to the negative of the constraint from
	// Gasse et al. Exact combinatorial optimization with graph convolutional neural networks 2019.
	auto const coefs = xvector({n_facilities}, 1.);
	for (std::size_t customer_idx = 0; customer_idx < n_customers; ++customer_idx) {
		auto const name = fmt::format("d_{}", customer_idx);
		auto cons = scip::create_cons_basic_linear(
			scip, name.c_str(), n_facilities, &serving_vars(customer_idx, 0), coefs.data(), 1.0, inf);
		scip::call(SCIPaddCons, scip, cons.get());
	}
}

/** Add n_facilities constraints stating that facilities cannot exceed their capacity.
 *
 * For each facility the sum of all fraction of demand served, multiplied by the demand, must be smaller than the
 * facility capacity.
 * Constraints are relased automatically (through unique_ptr in scip::create_cons_basic_linear).
 */
auto add_capacity_cons(
	SCIP* scip,
	xt::xtensor<SCIP_VAR*, 2> serving_vars,
	xt::xtensor<SCIP_VAR*, 1> const& facility_vars,
	xvector const& demands,
	xvector const& capacities) -> void {
	auto const inf = SCIPinfinity(scip);
	// Transposing and asserting row major as we pass the pointer as an array to SCIP when creating constraints.
	serving_vars = xt::transpose(std::move(serving_vars));
	assert(serving_vars.layout() == xt::layout_type::row_major);

	auto const [n_facilities, n_customers] = serving_vars.shape();
	assert(facility_vars.size() == n_facilities);
	assert(demands.size() == n_customers);
	assert(capacities.size() == n_facilities);

	for (std::size_t facility_idx = 0; facility_idx < n_facilities; ++facility_idx) {
		auto const name = fmt::format("c_{}", facility_idx);
		auto cons = scip::create_cons_basic_linear(
			scip, name.c_str(), n_customers, &serving_vars(facility_idx, 0), demands.data(), -inf, 0.);
		scip::call(SCIPaddCoefLinear, scip, cons.get(), facility_vars[facility_idx], -capacities[facility_idx]);
		scip::call(SCIPaddCons, scip, cons.get());
	}
}

/** Add n_customers * n_facilities constraint that tighten the LP relaxation.
 *
 * A facility cannot serve demand if it is not open.
 * Constraints are relased automatically (through unique_ptr in scip::create_cons_basic_linear).
 */
auto add_tightening_cons(
	SCIP* scip,
	xt::xtensor<SCIP_VAR*, 2> const& serving_vars,
	xt::xtensor<SCIP_VAR*, 1> const& facility_vars) -> void {
	auto const inf = SCIPinfinity(scip);
	auto const [n_customers, n_facilities] = serving_vars.shape();
	assert(facility_vars.size() == n_facilities);

	for (std::size_t customer_idx = 0; customer_idx < n_customers; ++customer_idx) {
		for (std::size_t facility_idx = 0; facility_idx < n_facilities; ++facility_idx) {
			auto const name = fmt::format("t_{}_{}", customer_idx, facility_idx);
			auto const vars = std::array{serving_vars(customer_idx, facility_idx), facility_vars[facility_idx]};
			auto constexpr coefs = std::array<scip::real, 2>{1., -1};
			auto cons = scip::create_cons_basic_linear(scip, name.c_str(), vars.size(), vars.data(), coefs.data(), -inf, 0.);
			scip::call(SCIPaddCons, scip, cons.get());
		}
	}
}

}  // namespace

scip::Model CapacitatedFacilityLocationGenerator::generate_instance(
	RandomEngine& random_engine,
	CapacitatedFacilityLocationGenerator::Parameters parameters) {

	// Why do we sample as ints?
	using xt::random::randint;

	// FIXME Should we keep these magic numbers or make them parameters?
	// Customer demand
	auto const demands = static_cast<xvector>(randint({parameters.n_customers}, 5, 35 + 1, random_engine));
	// Facilities capacity for serving customer demand
	auto capacities = static_cast<xvector>(randint({parameters.n_facilities}, 10, 160 + 1, random_engine));
	// Fixed costs for opening facilities
	auto const fixed_costs = static_cast<xvector>(
		randint({parameters.n_facilities}, 100, 110 + 1, random_engine) * xt::sqrt(capacities) +
		randint({parameters.n_facilities}, 0, 90 + 1, random_engine));
	// transport costs from facility to customers
	auto const transportation_costs = static_cast<xmatrix>(
		unit_transportation_costs(parameters.n_customers, parameters.n_facilities, random_engine) *
		xt::view(demands, xt::all(), xt::newaxis()));

	// Scale capacities according to ration after sampling as stated in Cornuejols et al. (1991).
	capacities = capacities * parameters.ratio * xt::sum(demands)() / xt::sum(capacities)();

	auto model = scip::Model::prob_basic();
	auto* const scip = model.get_scip_ptr();

	auto const facility_vars = add_facility_vars(scip, fixed_costs);
	auto const serving_vars = add_serving_vars(scip, transportation_costs);

	add_demand_cons(scip, serving_vars);
	add_capacity_cons(scip, serving_vars, facility_vars, demands, capacities);
	add_tightening_cons(scip, serving_vars, facility_vars);

	return model;
}

}  // namespace ecole::instance
