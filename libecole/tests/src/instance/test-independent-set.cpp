#include <algorithm>
#include <cstddef>
#include <map>

#include <catch2/catch.hpp>

#include "ecole/instance/independent-set.hpp"
#include "ecole/scip/cons.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;
using instance::IndependentSetGenerator;
using Params = instance::IndependentSetGenerator::Parameters;

TEST_CASE("IndependentSetGenerator unit test", "[unit][instance]") {
	// Keep problem size reasonable for tests
	std::size_t constexpr n_nodes = 100;
	instance::unit_tests(instance::IndependentSetGenerator{{n_nodes, Params::GraphType::erdos_renyi}});
	instance::unit_tests(instance::IndependentSetGenerator{{n_nodes, Params::GraphType::barabasi_albert}});
}

TEST_CASE("Instances generated are independent set instances", "[instance]") {
	std::size_t constexpr n_nodes = 100;
	auto const params =
		GENERATE(Params{n_nodes, Params::GraphType::erdos_renyi}, Params{n_nodes, Params::GraphType::barabasi_albert});
	auto generator = IndependentSetGenerator{params};
	auto model = generator.next();
	auto* const scip_ptr = model.get_scip_ptr();

	SECTION("Correct objective sense") { REQUIRE(SCIPgetObjsense(scip_ptr) == SCIP_OBJSENSE_MAXIMIZE); }

	SECTION("Correct variables type and count") {
		REQUIRE(model.variables().size() == n_nodes);
		for (auto* const var : model.variables()) {
			REQUIRE(SCIPvarGetType(var) == SCIP_VARTYPE_BINARY);
		}
	}

	SECTION("Correct constraints") {
		auto const inf = SCIPinfinity(scip_ptr);
		for (auto* const cons : model.constraints()) {
			// Correct constraints bounds
			REQUIRE(scip::cons_get_rhs(scip_ptr, cons).value() == 1.0);
			REQUIRE(scip::cons_get_lhs(scip_ptr, cons).value() == -inf);
			// Correct constraints coefficients
			auto const coefs = scip::get_vals_linear(scip_ptr, cons);
			REQUIRE(std::all_of(coefs.begin(), coefs.end(), [](auto coef) { return coef == 1.; }));
		}
	}

	SECTION("Each edge is used in only one constraint") {
		// Count each edge between two varaible in the constraints. Basically an adjacency list with counts.
		auto edge_counts = std::map<SCIP_VAR*, std::map<SCIP_VAR*, std::size_t>>{};
		for (auto* const cons : model.constraints()) {
			auto const vars = scip::get_vars_linear(scip_ptr, cons);
			// Iterating over all combinations of two variables (two nodes, i.e. an edge) in the constraint
			auto const* iter1 = vars.begin();
			while (iter1 != vars.end()) {
				auto const* iter2 = iter1 + 1;
				while (iter2 != vars.end()) {
					// Non existent keys are automatically added with default constructed values
					edge_counts[*iter1][*iter2]++;
					edge_counts[*iter2][*iter1]++;
					++iter2;
				}
				++iter1;
			}
		}

		// All variables seen at least once
		REQUIRE(edge_counts.size() == n_nodes);
		// Each pair of variables (edge) in the constraints exists only once
		for (auto const& [_, neighbors] : edge_counts) {
			REQUIRE(std::all_of(neighbors.begin(), neighbors.end(), [](auto key_val) { return key_val.second == 1; }));
		}
	}
}
