#include <cstddef>

#include <catch2/catch.hpp>
#include <scip/cons.h>
#include <scip/cons_linear.h>
#include <scip/scip.h>
#include <scip/var.h>

#include "ecole/instance/combinatorial-auction.hpp"
#include "ecole/scip/cons.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("CombinatorialAuctionGenerator unit tests", "[unit][instance]") {
	// Keep problem size reasonable for tests
	std::size_t constexpr n_items = 50;
	std::size_t constexpr n_bids = 50;
	instance::unit_tests(instance::CombinatorialAuctionGenerator{{n_items, n_bids}});
}

TEST_CASE("Instances generated are combinatorial auction instances", "[instance]") {

	auto constexpr params = instance::CombinatorialAuctionGenerator::Parameters{};
	auto generator = instance::CombinatorialAuctionGenerator{};
	auto model = generator.next();
	auto* const scip_ptr = model.get_scip_ptr();

	SECTION("Correct dimension and objective sense") {
		REQUIRE(model.variables().size() == params.n_bids);
		REQUIRE(SCIPgetObjsense(scip_ptr) == SCIP_OBJSENSE_MAXIMIZE);
	}

	SECTION("Variables are binary") {
		for (auto* const var : model.variables()) {
			REQUIRE(SCIPvarGetType(var) == SCIP_VARTYPE_BINARY);
		}
	}

	SECTION("Constraints contain only ones") {
		for (auto* const cons : model.constraints()) {
			auto const inf = SCIPinfinity(scip_ptr);
			REQUIRE(scip::cons_get_lhs(scip_ptr, cons).value() == -inf);
			REQUIRE(scip::cons_get_rhs(scip_ptr, cons).value() == 1.);
			for (auto const val : scip::get_vals_linear(scip_ptr, cons)) {
				REQUIRE(val == 1.);
			}
		}
	}
}
