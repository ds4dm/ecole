#include <catch2/catch.hpp>
#include <scip/cons.h>
#include <scip/cons_linear.h>
#include <scip/scip.h>
#include <scip/var.h>

#include "ecole/instance/set-cover.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("SetCoverGenerator unit test", "[unit][instance]") {
	instance::unit_tests(instance::SetCoverGenerator{});
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
			SCIP_Bool success = FALSE;
			REQUIRE(SCIPconsGetLhs(scip_ptr, cons, &success) == 1.);
			REQUIRE(success == TRUE);

			auto* const cons_values = SCIPgetValsLinear(scip_ptr, cons);
			auto const cons_n_values = SCIPgetNVarsLinear(scip_ptr, cons);
			for (auto i = 0; i < cons_n_values; ++i) {
				REQUIRE(cons_values[i] == 1.);
			}
		}
	}
}
