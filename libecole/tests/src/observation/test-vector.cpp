#include <type_traits>

#include <catch2/catch.hpp>

#include "ecole/observation/abstract.hpp"
#include "ecole/observation/vector.hpp"

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

TEST_CASE("Observation VectorFunction unit tests", "[unit][obs]") {
	observation::unit_tests(observation::VectorFunction{std::vector{IntObsFunc{}, IntObsFunc{}}});
}

TEST_CASE("Combine observation functions into a vector", "[obs]") {
	auto obs_func = observation::VectorFunction<IntObsFunc>{{{1}, {2}}};
	auto model = get_model();

	obs_func.reset(model);
	auto obs = obs_func.obtain_observation(model);
	STATIC_REQUIRE(std::is_same_v<decltype(obs), std::vector<int>>);
	REQUIRE(obs[0] == 2);
	REQUIRE(obs[1] == 3);
}
