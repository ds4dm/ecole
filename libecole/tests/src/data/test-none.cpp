#include <catch2/catch.hpp>

#include "ecole/data/none.hpp"

#include "conftest.hpp"
#include "data/unit-tests.hpp"

using namespace ecole::data;

TEST_CASE("NoneFunction unit tests", "[unit][data]") {
	unit_tests(NoneFunction{});
}

TEST_CASE("NoneFunction return None as data", "[data]") {
	auto const done = GENERATE(true, false);
	auto data_func = NoneFunction{};
	auto model = get_model();
	data_func.before_reset(model);
	advance_to_stage(model, SCIP_STAGE_SOLVING);

	REQUIRE(data_func.extract(model, done) == ecole::None);
}
