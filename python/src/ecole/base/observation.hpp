#pragma once

#include <memory>

#include <pybind11/pybind11.h>

#include "ecole/base.hpp"
#include "ecole/observation.hpp"

namespace py11 = pybind11;

namespace ecole {
namespace py {
namespace obs {
namespace internal {

/**
 * Base class for all observation.
 *
 * All observation must inherit from this class before being bound to Python in order to
 * be properly returned from environments.
 */
struct Py_ObsBase {
	virtual ~Py_ObsBase() = default;
};

/**
 * Wrapper to make C++ observation inherit form the base observation @ref Py_ObsBase.
 *
 * @tparam Obs The C++ observation to wrap.
 */
template <typename Obs> struct Py_Obs : public Py_ObsBase {
	Obs obs;
	Py_Obs(Obs&& obs) : obs(std::move(obs)) {}
};

/**
 * Base class for all observation spaces.
 *
 * All observation spaces must inherit from this class before being bound to Python in
 * order to be properly passed to environments.
 * Observations need to be returned as pointers for dynamic polymorphism.
 * These pointers need to be @ref std::shared_ptr and not @ref std::unique_ptr because
 * they can be created from Python when creating an observation space.
 */
using Py_ObsSpaceBase = base::ObservationSpace<std::shared_ptr<Py_ObsBase>>;

/**
 * Wrapper to make C++ observation spaces.
 *
 * Make the wrapped class properly inherit @ref Py_ObsSpaceBase and returned observation
 * in the proper way (using @ref std::shared_ptr).
 *
 * @tparam ObsSpace The C++ observation space to warp.
 */
template <typename ObsSpace> struct Py_ObsSpace : public Py_ObsSpaceBase {
	using obs_t = typename Py_ObsSpaceBase::obs_t;

	ObsSpace obs_space;

	template <typename... Args>
	Py_ObsSpace(Args... args) : obs_space(std::forward<Args>(args)...) {}

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<Py_ObsSpaceBase> clone() const override {
		return std::make_unique<Py_ObsSpace>(obs_space);
	}

	/**
	 * Move the observation from the wrapped @ref obs_space into a @ref std::shared_ptr
	 */
	obs_t get(scip::Model const& model) override {
		using py_obs_t = Py_Obs<typename ObsSpace::obs_t>;
		return std::make_shared<py_obs_t>(obs_space.get(model));
	}
};

/**
 * PyBind trampoline class for Python inheritance of @ref Py_ObsSpaceBase.
 *
 * Trampoline classes must inherit the type currently being bound and override all methods
 * to call `PYBIND_OVERLOAD_XXX`.
 * Every class needs its own trampoline with all override.
 *
 * @tparam PyObsSpace The type of @ref Py_ObsSpaceBase currently being bound (abstract or
 * derived).
 */
template <typename PyObsSpace> struct Py_ObsSpaceBase_Trampoline : public PyObsSpace {
	using typename Py_ObsSpaceBase::obs_t;

	using PyObsSpace::PyObsSpace;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<Py_ObsSpaceBase> clone() const override {
		return std::make_unique<Py_ObsSpaceBase_Trampoline>();
	}

	/**
	 * Override method to make it overridable from Python.
	 */
	void reset(scip::Model const& model) override {
		PYBIND11_OVERLOAD(void, PyObsSpace, reset, model);
	}

	/**
	 * Override pure method to make it overridable from Python.
	 */
	obs_t get(scip::Model const& model) override {
		PYBIND11_OVERLOAD_PURE(obs_t, PyObsSpace, get, model);
	}
};

/**
 * PyBind trampoline class for Python inheritance of vanilla @ ref Py_ObsSpace classes.
 *
 * Inherit override from @ref Py_ObsSpaceBase_Trampoline and override
 * @ref Py_ObsSpaceBase_Trampoline::get to non pure overload.
 * Similarily, if an observation space needs to make additional method overridable from
 * Python, it needs to define a new trampoline class (inheriting this one) with additional
 * overrides to call `PYBIND11_OVERLOAD_XXX`.
 *
 * @tparam PyObsSpace The type of @ref Py_ObsSpace currently being bound.
 */
template <typename PyObsSpace>
struct Py_ObsSpace_Trampoline : public Py_ObsSpaceBase_Trampoline<PyObsSpace> {
	using typename Py_ObsSpaceBase::obs_t;

	using Py_ObsSpaceBase_Trampoline<PyObsSpace>::Py_ObsSpaceBase_Trampoline;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<Py_ObsSpaceBase> clone() const override {
		return std::make_unique<Py_ObsSpace_Trampoline>();
	}

	/**
	 * Override no longer pure method to make default implemntation visible.
	 */
	obs_t get(scip::Model const& model) override {
		PYBIND11_OVERLOAD(obs_t, PyObsSpace, get, model);
	}
};

}  // namespace internal

/**
 * Aliases for Python observation and observation spaces classes.
 */
using ObsBase = internal::Py_ObsBase;
template <typename Observation> using Obs = internal::Py_Obs<Observation>;
using ObsSpaceBase = internal::Py_ObsSpaceBase;
template <typename ObservationSpace>
using ObsSpace = internal::Py_ObsSpace<ObservationSpace>;

/**
 * The @ref pybind11::class_ type for @ref ObsBase
 *
 * Set the holder type to @ref std::shared_ptr inheriting observation spaces prevent
 * from having it as a @ref std::unique_ptr.
 */
using base_obs_class_ = py11::class_<
	internal::Py_ObsBase,                  // Class
	std::shared_ptr<internal::Py_ObsBase>  // Holder
	>;

/**
 * The @ref pybind11::class_ type for all observation.
 *
 * Set the parent base class.
 * Set the holder type to @ref std::shared_ptr inheriting observation spaces prevent
 * from having it as a @ref std::unique_ptr.
 *
 * @tparam Observation The C++ observation to bind to Python
 */
template <typename Observation>
using obs_class_ = py11::class_<
	internal::Py_Obs<Observation>,                  // Class
	internal::Py_ObsBase,                           // Base
	std::shared_ptr<internal::Py_Obs<Observation>>  // Holder
	>;

/**
 * The @ref pybind11::class_ type for @ref base::ObservationSpace.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to be stored in environments).
 */
using base_space_class_ = py11::class_<
	internal::Py_ObsSpaceBase,                                        // Class
	internal::Py_ObsSpaceBase_Trampoline<internal::Py_ObsSpaceBase>,  // Trampoline
	std::shared_ptr<internal::Py_ObsSpaceBase>                        // Holder
	>;

/**
 * The @ref pybind11::class_ type for all observation spaces.
 *
 * Set the parent base class.
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to be stored in environments).
 *
 * @tparam ObsSapce The C++ observation space to bind to Python
 */
template <typename ObsSpace>
using space_class_ = py11::class_<
	internal::Py_ObsSpace<ObsSpace>,                                    // Class
	internal::Py_ObsSpaceBase,                                          // Base
	internal::Py_ObsSpace_Trampoline<internal::Py_ObsSpace<ObsSpace>>,  // Trampoline
	std::shared_ptr<internal::Py_ObsSpace<ObsSpace>>                    // Holder
	>;

}  // namespace obs
}  // namespace py
}  // namespace ecole
