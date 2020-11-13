#include <catch2/catch.hpp>

#include "ecole/observation/nothing.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

using namespace ecole;

TEST_CASE("Nothing unit tests", "[unit][obs]") {
	observation::unit_tests(observation::Nothing{});
}

TEST_CASE("Nothing return None as observation", "[obs]") {
	auto const done = GENERATE(true, false);
	auto obs_func = observation::Nothing{};
	auto model = get_model();
	obs_func.reset(model);

	REQUIRE(obs_func.extract(model, done) == ecole::None);
}
