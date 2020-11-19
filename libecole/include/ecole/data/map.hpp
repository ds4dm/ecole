#pragma once

#include <map>
#include <utility>

#include "ecole/data/abstract.hpp"
#include "ecole/traits.hpp"

namespace ecole::data {

/** Combine multiple data into a map of data. */
template <typename Key, typename Function>
class MapFunction : public DataFunction<std::map<Key, trait::data_of_t<Function>>> {
public:
	using DataMap = std::map<Key, trait::data_of_t<Function>>;

	/** Default construct all functions. */
	MapFunction() = default;

	/** Store a copy of the functions. */
	MapFunction(std::map<Key, Function> functions) : data_functions{std::move(functions)} {}

	/** Call before_reset on all functions. */
	void before_reset(scip::Model& model) override {
		for (auto& [_, func] : data_functions) {
			func.before_reset(model);
		}
	}

	/** Return data extracted from all functions as a map. */
	DataMap extract(scip::Model& model, bool done) override {
		auto data = DataMap{};
		for (auto& [key, func] : data_functions) {
			data.emplace_hint(data.end(), key, func.extract(model, done));
		}
		return data;
	}

private:
	std::map<Key, Function> data_functions;
};

}  // namespace ecole::data
