#pragma once

#include <map>
#include <utility>

#include "ecole/observation/abstract.hpp"
#include "ecole/traits.hpp"

namespace ecole::observation {

template <typename T> void show(T&&);

template <typename Key, typename Function>
class MapFunction : public ObservationFunction<std::map<Key, trait::observation_of_t<Function>>> {
public:
	using ObservationMap = std::map<Key, trait::observation_of_t<Function>>;

	/** Default construct all observation functions. */
	MapFunction() = default;

	/** Store a copy of the observation functions. */
	MapFunction(std::map<Key, Function> functions) : observation_functions{std::move(functions)} {}

	/** Call reset on all observation functions. */
	void reset(scip::Model& model) override {
		for (auto& [_, func] : observation_functions) {
			func.reset(model);
		}
	}

	/** Return observation from all functions as a map. */
	ObservationMap obtain_observation(scip::Model& model) override {
		auto obs = ObservationMap{};
		for (auto& [key, func] : observation_functions) {
			obs.emplace(key, func.obtain_observation(model));
		}
		return obs;
	}

private:
	std::map<Key, Function> observation_functions;
};

}  // namespace ecole::observation
