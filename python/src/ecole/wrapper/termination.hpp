#pragma once

#include <pybind11/pybind11.h>

#include "ecole/termination/base.hpp"

namespace py11 = pybind11;

namespace ecole {
namespace pytermination {

/*********************************
 *  Base and trampoline classes  *
 *********************************/

namespace internal {

/**
 * PyBind trampoline class for Python inheritance of termination::TerminationFunction.
 *
 * Trampoline classes must inherit the type currently being bound and override all methods
 * to call `PYBIND_OVERLOAD_XXX`.
 * Every class needs its own trampoline with all override.
 *
 * @tparam TerminationFunction The type of @ref termination::TerminationFunction
 * currently being bound (abstract or derived).
 */
template <typename TerminationFunction>
struct Py_TermFunctionBase_Trampoline : public TerminationFunction {
	using TerminationFunction::TerminationFunction;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<termination::TerminationFunction> clone() const override {
		return std::make_unique<Py_TermFunctionBase_Trampoline>();
	}

	/**
	 * Override method to make it overridable from Python.
	 */
	void reset(scip::Model const& model) override {
		PYBIND11_OVERLOAD(void, TerminationFunction, reset, model);
	}

	/**
	 * Override pure method to make it overridable from Python.
	 */
	bool is_done(scip::Model const& model) override {
		PYBIND11_OVERLOAD_PURE(bool, TerminationFunction, is_done, model);
	}
};

/**
 * PyBind trampoline class for Python inheritance of vanilla `TerminationFunctions`
 * classes.
 *
 * Inherit override from @ref PyRewardBase and override @ref PyRewardBase::get to non
 * pure overload.
 * Similarily, if a reward function needs to make additional method overridable from
 * Python, it needs to define a new trampoline class (inheriting this one) with additional
 * overrides to call `PYBIND11_OVERLOAD_XXX`.
 *
 * @tparam TerminationFunction The type of @ref termination::TerminationFunction
 * currently being bound.
 */
template <typename TerminationFunction>
struct Py_TermFunction_Trampoline :
	public Py_TermFunctionBase_Trampoline<TerminationFunction> {
	using Py_TermFunctionBase_Trampoline<
		TerminationFunction>::Py_TermFunctionBase_Trampoline;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<termination::TerminationFunction> clone() const override {
		return std::make_unique<Py_TermFunction_Trampoline>();
	}

	/**
	 * Override no longer pure method to make default implemntation visible.
	 */
	bool is_done(scip::Model const& model) override {
		PYBIND11_OVERLOAD(bool, TerminationFunction, is_done, model);
	}
};

}  // namespace internal

/*************************
 *  User facing aliases  *
 *************************/

/**
 * The @ref pybind11::class_ type for @ref termination::TerminationFunction.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to stored in environments).
 */
using base_func_class_ = py11::class_<
	termination::TerminationFunction,                                            // Class
	internal::Py_TermFunctionBase_Trampoline<termination::TerminationFunction>,  // Trampo
	std::shared_ptr<termination::TerminationFunction>                            // Holder
	>;

/**
 * The @ref pybind11::class_ type for reward functions.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to stored in environments).
 */
template <typename TerminationFunction>
using function_class_ = py11::class_<
	TerminationFunction,                                        // Class
	termination::TerminationFunction,                           // Base
	internal::Py_TermFunction_Trampoline<TerminationFunction>,  // Trampoline
	std::shared_ptr<TerminationFunction>                        // Holder
	>;

}  // namespace pytermination
}  // namespace ecole
