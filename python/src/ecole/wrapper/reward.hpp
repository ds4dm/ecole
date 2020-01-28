#pragma once

#include <pybind11/pybind11.h>

#include "ecole/reward/base.hpp"

namespace py11 = pybind11;

namespace ecole {
namespace pyreward {

/*********************************
 *  Base and trampoline classes  *
 *********************************/

namespace internal {

/**
 * PyBind trampoline class for Python inheritance of reward::RewardFunction.
 *
 * Trampoline classes must inherit the type currently being bound and override all methods
 * to call `PYBIND_OVERLOAD_XXX`.
 * Every class needs its own trampoline with all override.
 *
 * @tparam RewardFunction The type of @ref reward::RewardFunction currently being
 * bound (abstract or derived).
 */
template <typename RewardFunction>
struct Py_RewardFunctionBase_Trampoline : public RewardFunction {
	using typename reward::RewardFunction::reward_t;

	using RewardFunction::RewardFunction;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<reward::RewardFunction> clone() const override {
		return std::make_unique<Py_RewardFunctionBase_Trampoline>();
	}

	/**
	 * Override method to make it overridable from Python.
	 */
	void reset(scip::Model const& model) override {
		PYBIND11_OVERLOAD(void, RewardFunction, reset, model);
	}

	/**
	 * Override pure method to make it overridable from Python.
	 */
	reward_t get(scip::Model const& model, bool done = false) override {
		PYBIND11_OVERLOAD_PURE(reward_t, RewardFunction, get, model, done);
	}
};

/**
 * PyBind trampoline class for Python inheritance of vanilla `RewardFunctions` classes.
 *
 * Inherit override from @ref PyRewardBase and override @ref PyRewardBase::get to non
 * pure overload.
 * Similarily, if a reward function needs to make additional method overridable from
 * Python, it needs to define a new trampoline class (inheriting this one) with additional
 * overrides to call `PYBIND11_OVERLOAD_XXX`.
 *
 * @tparam RewardFunction The type of @ref reward::RewardFunction currently being
 * bound.
 */
template <typename RewardFunction>
struct Py_RewardFunction_Trampoline :
	public Py_RewardFunctionBase_Trampoline<RewardFunction> {
	using typename reward::RewardFunction::reward_t;

	using Py_RewardFunctionBase_Trampoline<
		RewardFunction>::Py_RewardFunctionBase_Trampoline;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<reward::RewardFunction> clone() const override {
		return std::make_unique<Py_RewardFunction_Trampoline>();
	}

	/**
	 * Override no longer pure method to make default implemntation visible.
	 */
	reward_t get(scip::Model const& model, bool done = false) override {
		PYBIND11_OVERLOAD(reward_t, RewardFunction, get, model, done);
	}
};

}  // namespace internal

/*************************
 *  User facing aliases  *
 *************************/

/**
 * The @ref pybind11::class_ type for @ref reward::RewardFunction.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to stored in environments).
 */
using base_func_class_ = py11::class_<
	reward::RewardFunction,                                              // Class
	internal::Py_RewardFunctionBase_Trampoline<reward::RewardFunction>,  // Trampoline
	std::shared_ptr<reward::RewardFunction>                              // Holder
	>;

/**
 * The @ref pybind11::class_ type for reward functions.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to stored in environments).
 */
template <typename RewardFunction>
using function_class_ = py11::class_<
	RewardFunction,                                          // Class
	reward::RewardFunction,                                  // Base
	internal::Py_RewardFunction_Trampoline<RewardFunction>,  // Trampoline
	std::shared_ptr<RewardFunction>                          // Holder
	>;

}  // namespace pyreward
}  // namespace ecole
