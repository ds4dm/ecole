#pragma once

#include <tuple>

#include "ecole/observation/abstract.hpp"
#include "ecole/traits.hpp"

namespace ecole::observation {

template <typename... Functions>
class TupleFunction : public ObservationFunction<std::tuple<trait::observation_of_t<Functions>...>> {
public:
	using ObservationTuple = std::tuple<trait::observation_of_t<Functions>...>;

	/** Default construct all observation functions. */
	TupleFunction() = default;

	/** Store a copy of the observation functions. */
	TupleFunction(Functions... functions) : observation_functions{std::move(functions)...} {}

	/** Call reset on all observation functions. */
	void reset(scip::Model& model) override {
		std::apply([&model](auto&... functions) { ((functions.reset(model)), ...); }, observation_functions);
	}

	/** Return observation from all functions as a tuple. */
	ObservationTuple obtain_observation(scip::Model& model, bool done) override {
		return std::apply(
			[&model, done](auto&... functions) { return std::tuple{functions.obtain_observation(model, done)...}; },
			observation_functions);
	}

private:
	std::tuple<Functions...> observation_functions;
};

}  // namespace ecole::observation
