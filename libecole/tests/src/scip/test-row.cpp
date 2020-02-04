#include <algorithm>

#include <catch2/catch.hpp>
#include <scip/scip.h>

#include "ecole/scip/model.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Model row vien iterator throw if not in solving stage") {
	REQUIRE_THROWS_AS(get_model().lp_rows(), scip::Exception);
}

TEST_CASE("Model has row view iterator") {
	auto model = get_model();
	std::function<void(scip::Model&)> test_during_branching;

	SECTION("with correct number of elements") {
		test_during_branching = [](auto& model) {
			auto const rows = model.lp_rows();
			auto const n_rows =
				std::count_if(rows.begin(), rows.end(), [](auto) { return true; });
			REQUIRE(n_rows == SCIPgetNLPRows(model.get_scip_ptr()));
		};
	}

	SECTION("Available during solving stage") {
		test_during_branching = [](auto& model) {
			auto count = 0;
			for (auto row : model.lp_rows())
				if (row.lhs() == 0) ++count;
			REQUIRE(count == 63);
		};
	}

	// Create a contex where lp_rows can be tested
	model.set_branch_rule([test_during_branching](auto& model) {
		test_during_branching(model);
		model.interrupt_solve();
		return scip::VarProxy::None;
	});
	model.solve();
}
