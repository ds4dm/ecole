#include <catch2/catch.hpp>

#include "ecole/scip/model.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Model has variable view iterator") {
	auto model = scip::Model::from_file(problem_file);
	int count = 0;
	for (auto var : model.variables()) {
		if (var.ub() == 1.)
			++count;
	}
	REQUIRE(count == 64);
}
