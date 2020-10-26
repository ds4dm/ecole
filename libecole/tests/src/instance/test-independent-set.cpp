#include <cstddef>

#include <catch2/catch.hpp>

#include "ecole/instance/independent-set.hpp"

#include "instance/unit-tests.hpp"

using namespace ecole;

TEST_CASE("IndependentSetGenerator unit test", "[unit][instance]") {
	// Keep problem size reasonable for tests
	std::size_t constexpr n_nodes = 100;
	using Parameters = instance::IndependentSetGenerator::Parameters;
	instance::unit_tests(instance::IndependentSetGenerator{{n_nodes, Parameters::erdos_renyi}});
	instance::unit_tests(instance::IndependentSetGenerator{{n_nodes, Parameters::barabasi_albert}});
}
