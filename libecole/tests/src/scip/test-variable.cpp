#include <catch2/catch.hpp>

#include "ecole/scip/model.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Model has variable view iterator") {
	auto model = get_model();
	int count = 0;
	for (auto var : model.variables()) {
		if (var.ub_local() == 1.) ++count;
	}
	REQUIRE(count == 64);
}
