#pragma once

#include <chrono>
#include <utility>

#include "ecole/data/abstract.hpp"
#include "ecole/utility/chrono.hpp"

namespace ecole::data {

namespace internal {

/** Time in seconds to execute the given function.
 *
 * FIXME Should it prevent compiler optimizations?
 * See example in https://github.com/facebook/folly/blob/master/folly/Benchmark.h
 */
template <typename Clock, typename Func> auto time(Func&& func) -> double {
	auto const start = Clock::now();
	func();
	auto const end = Clock::now();
	return std::chrono::duration<double>{end - start}.count();
}

}  // namespace internal

template <typename Function> class TimedFunction : public DataFunction<double> {
public:
	TimedFunction(Function func_, bool wall_ = false) : func{std::move(func_)}, wall{wall_} {}
	TimedFunction(bool wall_ = false) : wall{wall_} {}

	/** Reset the function being timed. **/
	auto before_reset(scip::Model& model) -> void override { func.before_reset(model); }

	/** Time the extract method of the function. **/
	auto extract(scip::Model& model, bool done) -> double override {
		if (wall) {
			return internal::time<std::chrono::steady_clock>([&]() { return func.extract(model, done); });
		}
		return internal::time<utility::cpu_clock>([&]() { return func.extract(model, done); });
	}

private:
	Function func{};
	bool wall = false;
};

}  // namespace ecole::data
