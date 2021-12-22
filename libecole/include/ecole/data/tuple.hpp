#pragma once

#include <tuple>

#include "ecole/traits.hpp"

namespace ecole::scip {
class Model;
}

namespace ecole::data {

template <typename... Functions> class TupleFunction {
public:
	using DataTuple = std::tuple<trait::data_of_t<Functions>...>;

	/** Default construct all functions. */
	TupleFunction() = default;

	/** Store a copy of the functions. */
	TupleFunction(Functions... functions) : data_functions{std::move(functions)...} {}
	TupleFunction(std::tuple<Functions...> functions) : data_functions{std::move(functions)} {}

	/** Call before_reset on all functions. */
	auto before_reset(scip::Model& model) -> void {
		std::apply([&model](auto&... functions) { ((functions.before_reset(model)), ...); }, data_functions);
	}

	/** Return data from all functions as a tuple. */
	auto extract(scip::Model& model, bool done) -> DataTuple {
		return std::apply(
			[&model, done](auto&... functions) { return std::tuple{functions.extract(model, done)...}; }, data_functions);
	}

private:
	std::tuple<Functions...> data_functions;
};

}  // namespace ecole::data
