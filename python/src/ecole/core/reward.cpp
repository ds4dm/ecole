#include <pybind11/eval.h>
#include <pybind11/pybind11.h>

#include "ecole/reward/constant.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/reward/lpiterations.hpp"

#include "core.hpp"

namespace ecole {
namespace reward {

namespace py = pybind11;

template <typename RewardFunction, typename... Args>
auto reward_function_class(py::module& m, Args&&... args) {
	return py::class_<RewardFunction>(m, std::forward<Args>(args)...)  //
		.def("reset", &RewardFunction::reset, py::arg("state"))
		.def(
			"obtain_reward",
			&RewardFunction::obtain_reward,
			py::arg("state"),
			py::arg("done") = false);
}

void bind_submodule(py::module m) {
	m.doc() = "Reward classes for Ecole.";

	// Import Python standrad modules
	auto const numbers = py::module::import("numbers");
	auto const builtins = py::module::import("builtins");
	auto const math = py::module::import("math");

	// Return a function that wraps rewards functions inside an ArithmeticReward.
	// The ArithmeticReward is a reward function class that will call the wrapped reward
	// functions and merge there rewards with the relevant operation (sum, prod, ...)
	auto const arithmetic_method = [m](auto operation) {
		auto const ArithmeticFunction = m.attr("ArithmeticFunction");
		return [ArithmeticFunction, operation](py::args args) {
			return ArithmeticFunction(operation, args);
		};
	};

	struct RewardFunction {};
	py::class_<RewardFunction> base_reward_function(m, "RewardFunction");
	base_reward_function
		// Binary operators
		.def("__add__", arithmetic_method(py::eval("lambda x, y: x + y")))
		.def("__sub__", arithmetic_method(py::eval("lambda x, y: x - y")))
		.def("__mul__", arithmetic_method(py::eval("lambda x, y: x * y")))
		.def("__matmul__", arithmetic_method(py::eval("lambda x, y: x @ y")))
		.def("__truediv__", arithmetic_method(py::eval("lambda x, y: x / y")))
		.def("__floordiv__", arithmetic_method(py::eval("lambda x, y: x // y")))
		.def("__mod__", arithmetic_method(py::eval("lambda x, y: x % y")))
		.def("__divmod__", arithmetic_method(builtins.attr("divmod")))
		.def("__pow__", arithmetic_method(builtins.attr("pow")))
		.def("__lshift__", arithmetic_method(py::eval("lambda x, y: x << y")))
		.def("__rshift__", arithmetic_method(py::eval("lambda x, y: x >> y")))
		.def("__and__", arithmetic_method(py::eval("lambda x, y: x & y")))
		.def("__xor__", arithmetic_method(py::eval("lambda x, y: x ^ y")))
		.def("__or__", arithmetic_method(py::eval("lambda x, y: x | y")))
		// Reversed binary operators
		.def("__radd__", arithmetic_method(py::eval("lambda x, y: y + x")))
		.def("__rsub__", arithmetic_method(py::eval("lambda x, y: y - x")))
		.def("__rmul__", arithmetic_method(py::eval("lambda x, y: y * x")))
		.def("__rmatmul__", arithmetic_method(py::eval("lambda x, y: y @ x")))
		.def("__rtruediv__", arithmetic_method(py::eval("lambda x, y: y / x")))
		.def("__rfloordiv__", arithmetic_method(py::eval("lambda x, y: y // x")))
		.def("__rmod__", arithmetic_method(py::eval("lambda x, y: y % x")))
		.def("__rdivmod__", arithmetic_method(py::eval("lambda x, y: divmod(y, x)")))
		.def("__rpow__", arithmetic_method(py::eval("lambda x, y: y ** x")))
		.def("__rlshift__", arithmetic_method(py::eval("lambda x, y: y << x")))
		.def("__rrshift__", arithmetic_method(py::eval("lambda x, y: y >> x")))
		.def("__rand__", arithmetic_method(py::eval("lambda x, y: y & x")))
		.def("__rxor__", arithmetic_method(py::eval("lambda x, y: y ^ x")))
		.def("__ror__", arithmetic_method(py::eval("lambda x, y: y | x")))
		// Unary operator
		.def("__neg__", arithmetic_method(py::eval("lambda x: -x")))
		.def("__pos__", arithmetic_method(py::eval("lambda x: +x")))
		.def("__abs__", arithmetic_method(builtins.attr("abs")))
		.def("__invert__", arithmetic_method(py::eval("lambda x: ~x")))
		.def("__int__", arithmetic_method(builtins.attr("int")))
		.def("__float__", arithmetic_method(builtins.attr("float")))
		.def("__complex__", arithmetic_method(builtins.attr("complex")))
		.def("__round__", arithmetic_method(builtins.attr("round")))
		.def("__trunc__", arithmetic_method(math.attr("trunc")))
		.def("__floor__", arithmetic_method(math.attr("floor")))
		.def("__ceil__", arithmetic_method(math.attr("ceil")));
	// Custom Math methods
	// clang-format off
	for (auto const name : {
		"sin", "exp", "log", "log2", "log10", "sqrt", "cos", "tan", "asin", "acos", "atan",
		"sinh", "cosh", "tanh", "asinh", "acosh", "atanh"
	}) {
		base_reward_function.def(name, arithmetic_method(math.attr(name)));
	}
	// clang-format on
	base_reward_function.def("apply", [m](py::object self, py::object func) {
		return m.attr("ArithmeticFunction")(func, py::make_tuple(self));
	});

	py::exec(
		R"(
		import numbers

		class ArithmeticFunction(RewardFunction):
			"""Apply an arithmetic operation on multiple rewards."""

			def __init__(self, operation, functions):
				self.operation = operation
				self.functions = [Constant(f) if isinstance(f, numbers.Number) else f for f in functions]

			def reset(self, initial_state):
				"""Call reset on all reward functions."""
				for obs_func in self.observation_functions:
					obs_func.reset(initial_state)

			def obtain_reward(self, state):
				return self.operation(*(f.obtain_reward(state) for f in self.functions))

			)",
		m.attr("__dict__"));

	reward_function_class<Constant>(m, "Constant", base_reward_function)  //
		.def(py::init<Reward>(), py::arg("constant") = 0.);

	reward_function_class<IsDone>(m, "IsDone", base_reward_function)  //
		.def(py::init<>());

	reward_function_class<LpIterations>(m, "LpIterations", base_reward_function)  //
		.def(py::init<>());
}

}  // namespace reward
}  // namespace ecole
