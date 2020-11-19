#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "ecole/data/abstract.hpp"
#include "ecole/traits.hpp"

namespace ecole::data {

/**
 * A class to map multiary operation onto data extraction functions.
 *
 * If the arity is one, then the operations are unary, such as exp(), sqrt(), and apply(...).
 * If the arity is two, then the operations are binary, such as + and *.
 */
template <typename DataCombiner, typename... Functions>
class MultiaryFunction : public DataFunction<std::invoke_result_t<DataCombiner, trait::data_of_t<Functions>...>> {
public:
	using CombinedData = std::invoke_result_t<DataCombiner, trait::data_of_t<Functions>...>;

	/** Default construct all functions. */
	MultiaryFunction() = default;

	/** Store a copy of all functions. */
	MultiaryFunction(DataCombiner combiner, Functions... functions) :
		data_functions{std::move(functions)...}, data_combiner{std::move(combiner)} {}

	/** Call before_reset on all functions. */
	void before_reset(scip::Model& model) override {
		std::apply([&model](auto&... functions) { ((functions.before_reset(model)), ...); }, data_functions);
	}

	/** Extract data from all functions and call the multiart operation on it. */
	CombinedData extract(scip::Model& model, bool done = false) override {
		return std::apply(
			[&](auto&... functions) { return data_combiner(functions.extract(model, done)...); }, data_functions);
	}

private:
	std::tuple<Functions...> data_functions;
	DataCombiner data_combiner;
};

/**
 * Alias for a single function.
 *
 * No type deduction before C++20.
 */
template <typename DataCombiner, typename Function> using UnaryFunction = MultiaryFunction<DataCombiner, Function>;

/**
 * Alias for two functions.
 *
 * No type deduction before C++20.
 */
template <typename DataCombiner, typename Function1, typename Function2>
using BinaryFunction = MultiaryFunction<DataCombiner, Function1, Function2>;

}  // namespace ecole::data
