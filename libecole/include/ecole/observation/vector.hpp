#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include "ecole/observation/abstract.hpp"
#include "ecole/traits.hpp"

namespace ecole::observation {

template <typename Function>
class VectorFunction : public ObservationFunction<std::vector<trait::observation_of_t<Function>>> {
public:
	using ObservationVector = std::vector<trait::observation_of_t<Function>>;

	/** Default construct all observation functions. */
	VectorFunction() = default;

	/** Store a copy of the observation functions. */
	VectorFunction(std::vector<Function> functions) : observation_functions{std::move(functions)} {}

	/** Call reset on all observation functions. */
	void reset(scip::Model& model) override {
		for (auto& func : observation_functions) {
			func.reset(model);
		}
	}

	/** Return observation from all functions as a vector. */
	ObservationVector extract(scip::Model& model, bool done) override {
		auto obs = ObservationVector(observation_functions.size());
		std::transform(observation_functions.begin(), observation_functions.end(), obs.begin(), [&model, done](auto& func) {
			return func.extract(model, done);
		});
		return obs;
	}

private:
	std::vector<Function> observation_functions;
};

}  // namespace ecole::observation
