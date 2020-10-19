#include <cstddef>

#include <catch2/catch.hpp>
#include <scip/cons.h>
#include <scip/cons_linear.h>
#include <scip/scip.h>
#include <scip/var.h>

#include "ecole/instance/set-cover.hpp"
#include "ecole/scip/cons.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("SetCoverGenerator unit test", "[unit][instance]") {
	// Keep problem size reasonable for tests
	std::size_t constexpr n_rows = 100;
	std::size_t constexpr n_cols = 200;
	instance::unit_tests(instance::SetCoverGenerator{{n_rows, n_cols}});
}

TEST_CASE("Instances generated are set cover instances", "[instance]") {
	auto constexpr params = instance::SetCoverGenerator::Parameters{};
	auto generator = instance::SetCoverGenerator{};
	auto model = generator.next();
	auto* const scip_ptr = model.get_scip_ptr();

	SECTION("Correct dimension and objective sense") {
		REQUIRE(model.variables().size() == params.n_cols);
		REQUIRE(model.constraints().size() == params.n_rows);
		REQUIRE(SCIPgetObjsense(scip_ptr) == SCIP_OBJSENSE_MINIMIZE);
	}

	SECTION("Variables are binary") {
		for (auto* const var : model.variables()) {
			REQUIRE(SCIPvarGetType(var) == SCIP_VARTYPE_BINARY);
		}
	}

	SECTION("Constraints contain only ones") {
		for (auto* const cons : model.constraints()) {
			auto const inf = SCIPinfinity(scip_ptr);
			REQUIRE(scip::cons_get_lhs(scip_ptr, cons).value() == 1.);
			REQUIRE(scip::cons_get_rhs(scip_ptr, cons).value() == inf);

			for (auto const val : scip::get_vals_linear(scip_ptr, cons)) {
				REQUIRE(val == 1.);
			}
		}
	}
}
