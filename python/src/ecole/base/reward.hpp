#pragma once

#include <pybind11/pybind11.h>

#include "ecole/base.hpp"
#include "ecole/reward.hpp"

namespace py11 = pybind11;

namespace ecole {
namespace py {
namespace reward {
namespace internal {

/**
 * PyBind trampoline class for Python inheritance of base::RewardSpace.
 *
 * Trampoline classes must inherit the type currently being bound and override all methods
 * to call `PYBIND_OVERLOAD_XXX`.
 * Every class needs its own trampoline with all override.
 *
 * @tparam RewardSpace The type of @ref base::RewardSpace currently being bound (abstract
 *     or derived).
 */
template <typename RewardSpace>
struct Py_RewardSpaceBase_Trampoline : public RewardSpace {
	using typename base::RewardSpace::reward_t;

	using RewardSpace::RewardSpace;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<base::RewardSpace> clone() const override {
		return std::make_unique<Py_RewardSpaceBase_Trampoline>();
	}

	/**
	 * Override method to make it overridable from Python.
	 */
	void reset(scip::Model const& model) override {
		PYBIND11_OVERLOAD(void, RewardSpace, reset, model);
	}

	/**
	 * Override pure method to make it overridable from Python.
	 */
	reward_t get(scip::Model const& model, bool done = false) override {
		PYBIND11_OVERLOAD_PURE(reward_t, RewardSpace, get, model, done);
	}
};

/**
 * PyBind trampoline class for Python inheritance of vanilla `RewardSpaces` classes.
 *
 * Inherit override from @ref PyRewardBase and override @ref PyRewardBase::get to non
 * pure overload.
 * Similarily, if a reward space needs to make additional method overridable from Python,
 * it needs to define a new trampoline class (inheriting this one) with additional
 * overrides to call `PYBIND11_OVERLOAD_XXX`.
 *
 * @tparam RewardSpace The type of @ref base::RewardSpace currently being bound.
 */
template <typename RewardSpace>
struct Py_RewardSpace_Trampoline : public Py_RewardSpaceBase_Trampoline<RewardSpace> {
	using typename base::RewardSpace::reward_t;

	using Py_RewardSpaceBase_Trampoline<RewardSpace>::Py_RewardSpaceBase_Trampoline;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<base::RewardSpace> clone() const override {
		return std::make_unique<Py_RewardSpace_Trampoline>();
	}

	/**
	 * Override no longer pure method to make default implemntation visible.
	 */
	reward_t get(scip::Model const& model, bool done = false) override {
		PYBIND11_OVERLOAD(reward_t, RewardSpace, get, model, done);
	}
};

}  // namespace internal

/**
 * The @ref pybind11::class_ type for @ref base::RewardSpace.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to stored in environments).
 */
using base_space_class_ = py11::class_<
	base::RewardSpace,                                           // Class
	internal::Py_RewardSpaceBase_Trampoline<base::RewardSpace>,  // Trampoline
	std::shared_ptr<base::RewardSpace>                           // Holder
	>;

/**
 * The @ref pybind11::class_ type for reward spaces.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to stored in environments).
 */
template <typename RewardSpace>
using space_class_ = py11::class_<
	RewardSpace,                                       // Class
	base::RewardSpace,                                 // Base
	internal::Py_RewardSpace_Trampoline<RewardSpace>,  // Trampoline
	std::shared_ptr<RewardSpace>                       // Holder
	>;

}  // namespace reward
}  // namespace py
}  // namespace ecole
