#include <algorithm>
#include <cmath>
#include <cstddef>

#include <catch2/catch.hpp>

#include "ecole/instance/capacitated-facility-location.hpp"
#include "ecole/scip/cons.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("CapaciteatedFacilityLocationGenerator unit test", "[unit][instance]") {
	// Keep problem size reasonable for tests
	std::size_t constexpr n_customers = 60;
	std::size_t constexpr n_facilities = 50;
	instance::unit_tests(instance::CapacitatedFacilityLocationGenerator{{n_customers, n_facilities}});
}

TEST_CASE("Instances generated are capacitated facility location instances", "[instance]") {
	auto constexpr params = instance::CapacitatedFacilityLocationGenerator::Parameters{};
	auto generator = instance::CapacitatedFacilityLocationGenerator{};
	auto model = generator.next();
	auto* const scip_ptr = model.get_scip_ptr();

	SECTION("Correct objective sense") { REQUIRE(SCIPgetObjsense(scip_ptr) == SCIP_OBJSENSE_MINIMIZE); }

	SECTION("Correct variables") {
		auto const vars = model.variables();
		auto const is_facility = [](auto* var) { return SCIPvarGetName(var)[0] == 'f'; };
		auto const is_serving = [](auto* var) { return SCIPvarGetName(var)[0] == 's'; };

		// Correct number of variables
		REQUIRE(std::count_if(vars.begin(), vars.end(), is_facility) == params.n_facilities);
		REQUIRE(std::count_if(vars.begin(), vars.end(), is_serving) == params.n_facilities * params.n_customers);

		// Correct variable type and bounds
		for (auto* var : vars) {
			if (is_facility(var)) {
				REQUIRE(SCIPvarGetType(var) == SCIP_VARTYPE_BINARY);
			} else if (is_serving(var)) {
				REQUIRE(SCIPvarGetType(var) == SCIP_VARTYPE_CONTINUOUS);
				REQUIRE(SCIPvarGetLbOriginal(var) == 0.0);
				REQUIRE(SCIPvarGetUbOriginal(var) == 1.0);
			}
		}
	}

	SECTION("Correct constraints") {
		auto const is_demand = [](auto* cons) { return SCIPconsGetName(cons)[0] == 'd'; };
		auto const is_capacity = [](auto* cons) { return SCIPconsGetName(cons)[0] == 'c'; };
		auto const is_thightening = [](auto* cons) { return SCIPconsGetName(cons)[0] == 't'; };
		auto const conss = model.constraints();

		// Correct number of constraints
		REQUIRE(std::count_if(conss.begin(), conss.end(), is_demand) == params.n_customers);
		REQUIRE(std::count_if(conss.begin(), conss.end(), is_capacity) == params.n_facilities);
		REQUIRE(std::count_if(conss.begin(), conss.end(), is_thightening) == params.n_facilities * params.n_customers);

		// Correct constraints bounds
		auto const inf = SCIPinfinity(scip_ptr);
		for (auto* cons : conss) {
			auto const coefs = scip::get_vals_linear(scip_ptr, cons);
			if (is_demand(cons)) {
				REQUIRE(scip::cons_get_lhs(scip_ptr, cons).value() == 1.0);
				REQUIRE(scip::cons_get_rhs(scip_ptr, cons).value() == inf);
				REQUIRE(coefs.size() == params.n_facilities);
				REQUIRE(std::all_of(coefs.begin(), coefs.end(), [](auto coef) { return coef == 1.; }));
			} else if (is_capacity(cons)) {
				REQUIRE(scip::cons_get_lhs(scip_ptr, cons).value() == -inf);
				REQUIRE(scip::cons_get_rhs(scip_ptr, cons).value() == 0.0);
				REQUIRE(coefs.size() == params.n_customers + 1);
			} else if (is_thightening(cons)) {
				REQUIRE(scip::cons_get_lhs(scip_ptr, cons).value() == -inf);
				REQUIRE(scip::cons_get_rhs(scip_ptr, cons).value() == 0.0);
				REQUIRE(coefs.size() == 2);
				REQUIRE(std::all_of(coefs.begin(), coefs.end(), [](auto coef) { return std::abs(coef) == 1.; }));
			}
		}
	}
}
