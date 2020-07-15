#include <catch2/catch.hpp>

#include "ecole/scip/model.hpp"

#include "conftest.hpp"

using namespace ecole;

TEST_CASE("Model has variable view iterator", "[scip]") {
	auto model = get_model();
	std::size_t count = 0;
	for (auto var : model.variables()) {
		count++;
		var.ub_local();
	}
	REQUIRE(count == model.variables().size);
}
