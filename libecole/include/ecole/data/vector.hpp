#pragma once

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "ecole/data/abstract.hpp"
#include "ecole/traits.hpp"

namespace ecole::data {

/* Combine multiple data into a vector of data. */
template <typename Function> class VectorFunction : public DataFunction<std::vector<trait::data_of_t<Function>>> {
public:
	using DataVector = std::vector<trait::data_of_t<Function>>;

	/** Default construct all functions. */
	VectorFunction() = default;

	/** Store a copy of the functions. */
	VectorFunction(std::vector<Function> functions) : data_functions{std::move(functions)} {}

	/** Call before_reset on all functions. */
	void before_reset(scip::Model& model) override {
		for (auto& func : data_functions) {
			func.before_reset(model);
		}
	}

	/** Return data extracted from all functions as a vector. */
	DataVector extract(scip::Model& model, bool done) override {
		auto data = DataVector{};
		data.reserve(data_functions.size());
		std::transform(data_functions.begin(), data_functions.end(), std::back_inserter(data), [&model, done](auto& func) {
			return func.extract(model, done);
		});
		return data;
	}

private:
	std::vector<Function> data_functions;
};

}  // namespace ecole::data
