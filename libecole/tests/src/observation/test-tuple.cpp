#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/observation/abstract.hpp"
#include "ecole/observation/tuple.hpp"

#include "conftest.hpp"
#include "observation/unit-tests.hpp"

using namespace ecole;

namespace ecole::observation {

/** Dummy Observation function to monitor what is happening. */
template <typename T> struct MockObservationFunction : ObservationFunction<T> {
	T val;

	MockObservationFunction() = default;
	MockObservationFunction(T val_) : val{val_} {}

	void reset(scip::Model& /* model */) override { ++val; };
	T obtain_observation(scip::Model& /* model */) override { return val; }
};

}  // namespace ecole::observation

using IntObsFunc = observation::MockObservationFunction<int>;
using DoubleObsFunc = observation::MockObservationFunction<double>;

TEST_CASE("Observation TupleFunction unit tests", "[unit][obs]") {
	observation::unit_tests(observation::TupleFunction{IntObsFunc{}, DoubleObsFunc{}});
}

TEST_CASE("Combine observation functions into a tuple", "[obs]") {
	auto obs_func = observation::TupleFunction{IntObsFunc{0}, DoubleObsFunc{1}};
	auto model = get_model();

	obs_func.reset(model);
	auto obs = obs_func.obtain_observation(model);
	STATIC_REQUIRE(std::is_same_v<decltype(obs), std::tuple<int, double>>);
	REQUIRE(std::get<0>(obs) == 1);
	REQUIRE(std::get<1>(obs) == 2.0);  // NOLINT(readability-magic-numbers)
}
