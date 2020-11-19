#include "ecole/data/abstract.hpp"

namespace ecole::data {

/** Dummy data function to monitor what is happening. */
template <typename T> struct MockFunction : DataFunction<T> {
	T val;

	MockFunction() = default;
	MockFunction(T val_) : val{val_} {}

	void before_reset(scip::Model& /* model */) override { ++val; };
	T extract(scip::Model& /* model */, bool /* done */) override { return val; }
};

using IntDataFunc = MockFunction<int>;
using DoubleDataFunc = MockFunction<double>;

}  // namespace ecole::data
