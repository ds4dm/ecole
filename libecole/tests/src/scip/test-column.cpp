#include <catch2/catch.hpp>

#include "ecole/scip/model.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Model has column view iterator") {
	auto model = scip::Model::from_file(problem_file);

	SECTION("Throw if not in solving stage") {
		REQUIRE_THROWS_AS(model.lp_columns(), scip::Exception);
	}

	SECTION("Available during solving stage") {
		auto test_lp_columns = [](auto& model) {
			int count = 0;
			for (auto col : model.lp_columns()) {
				if (col.ub() == 1.) ++count;
			}
			REQUIRE(count == 64);
		};

		// Create a contex where lp_columns can be tested
		model.set_branch_rule([test_lp_columns](auto& model) {
			test_lp_columns(model);
			model.interrupt_solve();
			return scip::VarProxy::None;
		});
		model.solve();
	}
}
