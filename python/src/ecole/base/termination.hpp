#pragma once

#include <pybind11/pybind11.h>

#include "ecole/base.hpp"
#include "ecole/termination.hpp"

namespace py11 = pybind11;

namespace ecole {
namespace py {
namespace termination {
namespace internal {

/**
 * PyBind trampoline class for Python inheritance of base::TerminationSpace.
 *
 * Trampoline classes must inherit the type currently being bound and override all methods
 * to call `PYBIND_OVERLOAD_XXX`.
 * Every class needs its own trampoline with all override.
 *
 * @tparam TerminationSpace The type of @ref base::TerminationSpace currently being bound
 * (abstract or derived).
 */
template <typename TerminationSpace>
struct Py_TermSpaceBase_Trampoline : public TerminationSpace {
	using TerminationSpace::TerminationSpace;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<base::TerminationSpace> clone() const override {
		return std::make_unique<Py_TermSpaceBase_Trampoline>();
	}

	/**
	 * Override method to make it overridable from Python.
	 */
	void reset(scip::Model const& model) override {
		PYBIND11_OVERLOAD(void, TerminationSpace, reset, model);
	}

	/**
	 * Override pure method to make it overridable from Python.
	 */
	bool is_done(scip::Model const& model) override {
		PYBIND11_OVERLOAD_PURE(bool, TerminationSpace, is_done, model);
	}
};

/**
 * PyBind trampoline class for Python inheritance of vanilla `TerminationSpaces` classes.
 *
 * Inherit override from @ref PyRewardBase and override @ref PyRewardBase::get to non
 * pure overload.
 * Similarily, if a reward space needs to make additional method overridable from Python,
 * it needs to define a new trampoline class (inheriting this one) with additional
 * overrides to call `PYBIND11_OVERLOAD_XXX`.
 *
 * @tparam TerminationSpace The type of @ref base::TerminationSpace currently being bound.
 */
template <typename TerminationSpace>
struct Py_TermSpace_Trampoline : public Py_TermSpaceBase_Trampoline<TerminationSpace> {
	using Py_TermSpaceBase_Trampoline<TerminationSpace>::Py_TermSpaceBase_Trampoline;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<base::TerminationSpace> clone() const override {
		return std::make_unique<Py_TermSpace_Trampoline>();
	}

	/**
	 * Override no longer pure method to make default implemntation visible.
	 */
	bool is_done(scip::Model const& model) override {
		PYBIND11_OVERLOAD(bool, TerminationSpace, is_done, model);
	}
};

}  // namespace internal

/**
 * The @ref pybind11::class_ type for @ref base::TerminationSpace.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to stored in environments).
 */
using base_space_class_ = py11::class_<
	base::TerminationSpace,                                         // Class
	internal::Py_TermSpaceBase_Trampoline<base::TerminationSpace>,  // Trampoline
	std::shared_ptr<base::TerminationSpace>                         // Holder
	>;

/**
 * The @ref pybind11::class_ type for reward spaces.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to stored in environments).
 */
template <typename TerminationSpace>
using space_class_ = py11::class_<
	TerminationSpace,                                     // Class
	base::TerminationSpace,                               // Base
	internal::Py_TermSpace_Trampoline<TerminationSpace>,  // Trampoline
	std::shared_ptr<TerminationSpace>                     // Holder
	>;

}  // namespace termination
}  // namespace py
}  // namespace ecole
