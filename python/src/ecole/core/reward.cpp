#include <pybind11/eval.h>
#include <pybind11/pybind11.h>

#include "ecole/reward/constant.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/reward/lpiterations.hpp"
#include "ecole/reward/nnodes.hpp"
#include "ecole/scip/model.hpp"

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
class Arithmetic {
public:
	Arithmetic(py::object operation, py::list functions, py::str repr);
	void reset(py::object model);
	Reward obtain_reward(py::object model, bool done);
	py::str toString() const;

private:
	py::object operation;
	py::list functions;
	py::str repr;
};

class Cumulative {
public:
	Cumulative(py::object function, py::object reduce_func, Reward inti_cumul, py::str repr);
	void reset(py::object model);
	Reward obtain_reward(py::object model, bool done);
	py::str toString() const;

private:
	py::object reduce_func;
	py::object function;
	Reward init_cumul;
	Reward cumul;
	py::str repr;
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

		Always returns the value passed in constructor.
	)");
	constant  //
		.def(py::init<Reward>(), py::arg("constant") = 0.)
		.def_readonly("constant", &Constant::constant);
	def_operators(constant);
	def_reset(constant, "Do nothing.");
	def_obtain_reward(constant, "Return the constant value.");

	auto arithmetic = py::class_<Arithmetic>(m, "Arithmetic", R"(
		Proxy class for doing arithmetic on reward functions.

		An object of this class is returned by reward function operators to forward calls
		to the reward function parameters of the operator.
	)");
	arithmetic  //
		.def(py::init<py::object, py::list, py::str>())
		.def("__repr__", &Arithmetic::toString);
	def_operators(arithmetic);
	def_reset(arithmetic, R"(
		Reset the reward functions of the operator.

		Calls ``reset`` on all reward functions parameters that were used to create this
		object.
	)");
	def_obtain_reward(arithmetic, R"(
		Obtain the reward of result of the operator.

		Calls ``obtain_reward`` on all reward function parameters that were used to create
		this object and compute the operation on the results.
	)");

	auto cumulative = py::class_<Cumulative>(m, "Cumulative", R"(
		Proxy class for doing cumulating reward throughout an episode.

		An object of this class is returned by reward functions cumulative operations to forward call
		to the reward function and apply a reduce function.
	)");
	cumulative  //
		.def(py::init<py::object, py::object, Reward, py::str>())
		.def("__repr__", &Cumulative::toString);
	def_operators(cumulative);
	def_reset(cumulative, "Reset the wrapped reward function and reset current cumulation.");
	def_obtain_reward(cumulative, "Obtain the cumulative reward of result of wrapped function.");

	auto isdone = py::class_<IsDone>(m, "IsDone", "Single reward on terminal states.");
	isdone.def(py::init<>());
	def_operators(isdone);
	def_reset(isdone, "Do nothing.");
	def_obtain_reward(isdone, "Return 1 if the episode is on a terminal state, 0 otherwise.");

	auto lpiterations = py::class_<LpIterations>(m, "LpIterations", R"(
		LP Iteration difference.

		The reward is defined as the number of iterations spent in solving the Linear Programs
		associated with the problem since the previous state.
	)");
	lpiterations.def(py::init<>());
	def_operators(lpiterations);
	def_reset(lpiterations, "Reset the internal LP iterations count.");
	def_obtain_reward(lpiterations, R"(
		Update the internal LP iteration count and return the difference.

		The difference in LP iterations is computed in between calls.
		)");

	auto nnodes = py::class_<NNodes>(m, "NNodes", R"(
		Number of nodes difference.

		The reward is defined as the total number of nodes processed since the previous state.
	)");
	nnodes.def(py::init<>());
	def_operators(nnodes);
	def_reset(nnodes, "Reset the internal node count.");
	def_obtain_reward(nnodes, R"(
		Update the internal node count and return the difference.

		The difference in number of nodes is computed in between calls.
		)");
}

/******************************
 *  Definition of Arithmetic  *
 ******************************/

Arithmetic::Arithmetic(py::object operation_, py::list functions_, py::str repr_) :
	operation(std::move(operation_)), repr(std::move(repr_)) {
	auto const Numbers = py::module::import("numbers").attr("Number");
	for (auto func : functions_) {
		if (py::isinstance(func, Numbers)) {
			functions.append(py::cast(Constant{func.cast<Reward>()}));
		} else {
			functions.append(func);
		}
	}
}

void Arithmetic::reset(py::object model) {
	for (auto obs_func : functions) {
		obs_func.attr("reset")(model);
	}
}

Reward Arithmetic::obtain_reward(py::object model, bool done) {
	py::list rewards{};
	for (auto obs_func : functions) {
		rewards.append(obs_func.attr("obtain_reward")(model, done));
	}
	return operation(*rewards).cast<Reward>();
}

py::str Arithmetic::toString() const {
	return repr.format(*functions);
}

/******************************
 *  Definition of Cumulative  *
 ******************************/

Cumulative::Cumulative(
	py::object function_,
	py::object reduce_func_,
	Reward init_cumul_,
	py::str repr_) :
	reduce_func(std::move(reduce_func_)),
	function(std::move(function_)),
	init_cumul(init_cumul_),
	cumul(init_cumul_),
	repr(std::move(repr_)) {}

void Cumulative::reset(py::object model) {
	cumul = init_cumul;
	function.attr("reset")(std::move(model));
}

Reward Cumulative::obtain_reward(py::object model, bool done) {
	auto reward = function.attr("obtain_reward")(std::move(model), done);
	cumul = reduce_func(py::cast(cumul), reward).cast<Reward>();
	return cumul;
}

py::str Cumulative::toString() const {
	return repr.format(function);
}

/************************************
 *  Definition of helper functions  *
 ************************************/

template <typename PyClass, typename... Args> void def_reset(PyClass pyclass, Args&&... args) {
	pyclass.def("reset", &PyClass::type::reset, py::arg("model"), std::forward<Args>(args)...);
}

template <typename PyClass, typename... Args>
void def_obtain_reward(PyClass pyclass, Args&&... args) {
	pyclass.def(
		"obtain_reward",
		&PyClass::type::obtain_reward,
		py::arg("model"),
		py::arg("done") = false,
		std::forward<Args>(args)...);
}

template <typename PyClass> void def_operators(PyClass pyclass) {
	// Import Python standrad modules
	auto const builtins = py::module::import("builtins");
	auto const math = py::module::import("math");

	// Return a function that wraps rewards functions inside an ArithmeticReward.
	// The Arithmetic reward function is a reward function class that will call the wrapped
	// reward functions and merge there rewards with the relevant operation (sum, prod, ...)
	auto const arith_meth = [](auto operation, auto repr) {
		return [operation, repr](py::args args) { return Arithmetic{operation, args, repr}; };
	};

	pyclass
		// Binary operators
		.def("__add__", arith_meth(py::eval("lambda x, y: x + y"), "({} + {})"))
		.def("__sub__", arith_meth(py::eval("lambda x, y: x - y"), "({} - {})"))
		.def("__mul__", arith_meth(py::eval("lambda x, y: x * y"), "({} * {})"))
		.def("__matmul__", arith_meth(py::eval("lambda x, y: x @ y"), "({} @ {})"))
		.def("__truediv__", arith_meth(py::eval("lambda x, y: x / y"), "({} / {})"))
		.def("__floordiv__", arith_meth(py::eval("lambda x, y: x // y"), "({} // {})"))
		.def("__mod__", arith_meth(py::eval("lambda x, y: x % y"), "({} % {})"))
		.def("__divmod__", arith_meth(builtins.attr("divmod"), "divmod({}, {})"))
		.def("__pow__", arith_meth(builtins.attr("pow"), "({} ** {})"))
		.def("__lshift__", arith_meth(py::eval("lambda x, y: x << y"), "({} << {})"))
		.def("__rshift__", arith_meth(py::eval("lambda x, y: x >> y"), "({} >> {})"))
		.def("__and__", arith_meth(py::eval("lambda x, y: x & y"), "({} & {})"))
		.def("__xor__", arith_meth(py::eval("lambda x, y: x ^ y"), "({} ^ {})"))
		.def("__or__", arith_meth(py::eval("lambda x, y: x | y"), "({} | {})"))
		// Reversed binary operators
		.def("__radd__", arith_meth(py::eval("lambda x, y: y + x"), "({1} + {0})"))
		.def("__rsub__", arith_meth(py::eval("lambda x, y: y - x"), "({1} - {0})"))
		.def("__rmul__", arith_meth(py::eval("lambda x, y: y * x"), "({1} * {0})"))
		.def("__rmatmul__", arith_meth(py::eval("lambda x, y: y @ x"), "({1} @ {0})"))
		.def("__rtruediv__", arith_meth(py::eval("lambda x, y: y / x"), "({1} / {0})"))
		.def("__rfloordiv__", arith_meth(py::eval("lambda x, y: y // x"), "({1} // {0})"))
		.def("__rmod__", arith_meth(py::eval("lambda x, y: y % x"), "({1} % {0})"))
		.def("__rdivmod__", arith_meth(py::eval("lambda x, y: divmod(y, x)"), "divmod({1}, {0})"))
		.def("__rpow__", arith_meth(py::eval("lambda x, y: y ** x"), "({1} ** {0})"))
		.def("__rlshift__", arith_meth(py::eval("lambda x, y: y << x"), "({1} << {0})"))
		.def("__rrshift__", arith_meth(py::eval("lambda x, y: y >> x"), "({1} >> {0})"))
		.def("__rand__", arith_meth(py::eval("lambda x, y: y & x"), "({1} & {0})"))
		.def("__rxor__", arith_meth(py::eval("lambda x, y: y ^ x"), "({1} ^ {0})"))
		.def("__ror__", arith_meth(py::eval("lambda x, y: y | x"), "({1} | {0})"))
		// Unary operator
		.def("__neg__", arith_meth(py::eval("lambda x: -x"), "(-{})"))
		.def("__pos__", arith_meth(py::eval("lambda x: +x"), "(+{})"))
		.def("__abs__", arith_meth(builtins.attr("abs"), "(abs({}))"))
		.def("__invert__", arith_meth(py::eval("lambda x: ~x"), "(~{})"))
		.def("__int__", arith_meth(builtins.attr("int"), "int({})"))
		.def("__float__", arith_meth(builtins.attr("float"), "float({})"))
		.def("__complex__", arith_meth(builtins.attr("complex"), "complex({})"))
		.def("__round__", arith_meth(builtins.attr("round"), "round({})"))
		.def("__trunc__", arith_meth(math.attr("trunc"), "math.trunc({})"))
		.def("__floor__", arith_meth(math.attr("floor"), "math.floor({})"))
		.def("__ceil__", arith_meth(math.attr("ceil"), "math.ceil({})"));
	// Custom Math methods
	// clang-format off
	for (auto const name : {
		"exp", "log", "log2", "log10", "sqrt", "sin", "cos", "tan", "asin", "acos", "atan",
		"sinh", "cosh", "tanh", "asinh", "acosh", "atanh", "isfinite", "isinf", "isnan"
	}) {
		pyclass.def(name, arith_meth(math.attr(name), std::string{"{}."} + name + "()"));
	}
	// clang-format on
	pyclass.def("apply", [](py::object self, py::object func) {
		return Arithmetic{func, py::make_tuple(self), "lambda({})"};
	});
	// Cumulative methods
	pyclass.def("cumsum", [](py::object self) {
		return Cumulative{self, py::eval("lambda x, y: x + y"), 0., "{}.cumsum()"};
	});
}

}  // namespace reward
}  // namespace ecole
