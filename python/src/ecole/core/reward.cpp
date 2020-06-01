#include <pybind11/eval.h>
#include <pybind11/pybind11.h>

#include "ecole/reward/constant.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/reward/lpiterations.hpp"

#include "core.hpp"

namespace py = pybind11;

namespace ecole {
namespace reward {

/**
 * Proxy class for doing arithmetic on reward functions.
 *
 * This could be a pure Python class, but it needs to be defined in this module to
 * be accessible to reward functions operators.
 */
struct ArithmeticFunction {
	py::object operation;
	py::list functions;

	ArithmeticFunction(py::object operation, py::list functions);
	void reset(py::object initial_state);
	Reward obtain_reward(py::object state);
};

/**
 * Helper function to bind common methods.
 */
template <typename PyClass, typename... Args> void def_reset(PyClass, Args&&...);
template <typename PyClass, typename... Args> void def_obtain_reward(PyClass, Args&&...);
template <typename PyClass> void def_operators(PyClass);

/**
 * Reward module bindings definitions.
 */
void bind_submodule(py::module m) {
	m.doc() = "Reward classes for Ecole.";

	auto constant = py::class_<Constant>(m, "Constant", R"(
		Constant Reward.

		Always return the value passed in constructor.
	)");
	constant  //
		.def(py::init<Reward>(), py::arg("constant") = 0.)
		.def_readonly("constant", &Constant::constant);
	def_operators(constant);
	def_reset(constant, "Do nothing.");
	def_obtain_reward(constant, "Return the constant value.");

	auto arithmetic_function = py::class_<ArithmeticFunction>(
		m,
		"ArithmeticFunction"
		R"(
		Proxy class for doing arithmetic on reward functions.

		An object of this class is returned by reward functions operators to forward calls
		to the reward functions parameters of the operator.
	)");
	arithmetic_function.def(py::init<py::object, py::list>());
	def_operators(arithmetic_function);
	def_reset(arithmetic_function, R"(
		Reset the reward functions of the operator.

		Call ``reset`` on all reward functions parameters that were used to create this
		object.
	)");
	def_obtain_reward(arithmetic_function, R"(
		Obtain the reward of result of the operator.

		Call ``obtain_reward`` on all reward functions parameters that were used to create
		this object and compute the operation on the results.
	)");

	auto isdone = py::class_<IsDone>(m, "IsDone", "Single reward on terminal states.");
	isdone.def(py::init<>());
	def_operators(isdone);
	def_reset(isdone, "Do nothing.");
	def_obtain_reward(
		isdone, "Return 1 if the episode is on a terminal state, 0 otherwise.");

	auto lpiterations = py::class_<LpIterations>(m, "LpIterations", R"(
		LP Iteration difference.

		The reward is defined as the number of iterations done solving the Linear Program
		associated with the problem since the previous state.
	)");
	lpiterations.def(py::init<>());
	def_operators(lpiterations);
	def_reset(lpiterations, "Reset the internal LP iterations count.");
	def_obtain_reward(lpiterations, R"(
		Update the internal LP iteration count and return the difference.

		The difference in LP iteration is computed in between calls.
		)");
}

/**************************************
 *  Definition of ArithmeticFunction  *
 **************************************/

ArithmeticFunction::ArithmeticFunction(py::object operation_, py::list functions_) :
	operation(std::move(operation_)) {
	auto const Numbers = py::module::import("numbers").attr("Number");
	for (auto func : functions_) {
		if (py::isinstance(func, Numbers)) {
			functions.append(py::cast(Constant{func.cast<Reward>()}));
		} else {
			functions.append(func);
		}
	}
}

void ArithmeticFunction::reset(py::object initial_state) {
	for (auto obs_func : functions) {
		obs_func.attr("reset")(initial_state);
	}
}

Reward ArithmeticFunction::obtain_reward(py::object state) {
	py::list rewards{};
	for (auto obs_func : functions) {
		rewards.append(obs_func.attr("obtain_reward")(state));
	}
	return operation(*rewards).cast<Reward>();
}

/************************************
 *  Definition of helper functions  *
 ************************************/

template <typename PyClass, typename... Args>
void def_reset(PyClass pyclass, Args&&... args) {
	pyclass.def(
		"reset", &PyClass::type::reset, py::arg("state"), std::forward<Args>(args)...);
}

template <typename PyClass, typename... Args>
void def_obtain_reward(PyClass pyclass, Args&&... args) {
	pyclass.def(
		"obtain_reward",
		&PyClass::type::obtain_reward,
		py::arg("state"),
		py::arg("done") = false,
		std::forward<Args>(args)...);
}

template <typename PyClass> void def_operators(PyClass pyclass) {
	// Import Python standrad modules
	auto const builtins = py::module::import("builtins");
	auto const math = py::module::import("math");

	// Return a function that wraps rewards functions inside an ArithmeticReward.
	// The ArithmeticReward is a reward function class that will call the wrapped reward
	// functions and merge there rewards with the relevant operation (sum, prod, ...)
	auto const arithmetic_method = [](auto operation) {
		return [operation](py::args args) { return ArithmeticFunction{operation, args}; };
	};

	pyclass
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
		pyclass.def(name, arithmetic_method(math.attr(name)));
	}
	// clang-format on
	pyclass.def("apply", [](py::object self, py::object func) {
		return ArithmeticFunction{func, py::make_tuple(self)};
	});
}

}  // namespace reward
}  // namespace ecole
