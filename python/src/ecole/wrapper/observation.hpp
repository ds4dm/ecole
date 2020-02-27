#pragma once

#include <memory>

#include <pybind11/pybind11.h>

#include "ecole/observation/abstract.hpp"

#include "container.hpp"

namespace py11 = pybind11;

namespace ecole {
namespace pyobservation {

/*********************************
 *  Base and trampoline classes  *
 *********************************/

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
 * Wrapper to make C++ observation inherit form the abstract observation @ref Py_ObsBase.
 *
 * @tparam Obs The C++ observation to wrap.
 */
template <typename Obs> struct Py_Obs : public Py_ObsBase {
	Obs obs;
	Py_Obs(Obs&& obs) : obs(std::move(obs)) {}
};

/**
 * Base class for all observation functions.
 *
 * All observation functions must inherit from this class before being bound to Python in
 * order to be properly passed to environments.
 * Observations need to be returned as pointers for dynamic polymorphism.
 * These pointers need to be @ref std::shared_ptr and not @ref std::unique_ptr because
 * they can be created from Python when creating an observation function.
 */
using Py_ObsFunctionBase = observation::ObservationFunction<std::shared_ptr<Py_ObsBase>>;

/**
 * Wrapper to make C++ observation functions.
 *
 * Make the wrapped class properly inherit @ref Py_ObsFunctionBase and returned
 * observation in the proper way (using @ref std::shared_ptr).
 *
 * @tparam ObsFunction The C++ observation function to warp.
 */
template <typename ObsFunction> struct Py_ObsFunction : public Py_ObsFunctionBase {
	using Observation = typename Py_ObsFunctionBase::Observation;

	ObsFunction obs_func;

	template <typename... Args>
	Py_ObsFunction(Args... args) : obs_func(std::forward<Args>(args)...) {}

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<Py_ObsFunctionBase> clone() const override {
		return std::make_unique<Py_ObsFunction>(obs_func);
	}

	/**
	 * Move the observation from the wrapped @ref obs_func into a @ref std::shared_ptr
	 */
	Observation get(environment::State const& state) override {
		using py_obs_t = Py_Obs<typename ObsFunction::Observation>;
		return std::make_shared<py_obs_t>(obs_func.get(state));
	}
};

/**
 * PyBind trampoline class for Python inheritance of @ref Py_ObsFunctionBase.
 *
 * Trampoline classes must inherit the type currently being bound and override all methods
 * to call `PYBIND_OVERLOAD_XXX`.
 * Every class needs its own trampoline with all override.
 *
 * @tparam PyObsFunction The type of @ref Py_ObsFunctionBase currently being bound
 * (abstract or derived).
 */
template <typename PyObsFunction>
struct Py_ObsFunctionBase_Trampoline : public PyObsFunction {
	using typename Py_ObsFunctionBase::Observation;

	using PyObsFunction::PyObsFunction;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<Py_ObsFunctionBase> clone() const override {
		return std::make_unique<Py_ObsFunctionBase_Trampoline>();
	}

	/**
	 * Override method to make it overridable from Python.
	 */
	void reset(environment::State const& init_state) override {
		PYBIND11_OVERLOAD(void, PyObsFunction, reset, init_state);
	}

	/**
	 * Override pure method to make it overridable from Python.
	 */
	Observation get(environment::State const& state) override {
		PYBIND11_OVERLOAD_PURE(Observation, PyObsFunction, get, state);
	}
};

/**
 * PyBind trampoline class for Python inheritance of vanilla @ ref Py_ObsFunction classes.
 *
 * Inherit override from @ref Py_ObsFunctionBase_Trampoline and override
 * @ref Py_ObsFunctionBase_Trampoline::get to non pure overload.
 * Similarily, if an observation function needs to make additional method overridable from
 * Python, it needs to define a new trampoline class (inheriting this one) with additional
 * overrides to call `PYBIND11_OVERLOAD_XXX`.
 *
 * @tparam PyObsFunction The type of @ref Py_ObsFunction currently being bound.
 */
template <typename PyObsFunction>
struct Py_ObsFunction_Trampoline : public Py_ObsFunctionBase_Trampoline<PyObsFunction> {
	using typename Py_ObsFunctionBase::Observation;

	using Py_ObsFunctionBase_Trampoline<PyObsFunction>::Py_ObsFunctionBase_Trampoline;

	/**
	 * Implement clone method required to make the class non pure abstract for C++ end.
	 */
	std::unique_ptr<Py_ObsFunctionBase> clone() const override {
		return std::make_unique<Py_ObsFunction_Trampoline>();
	}

	/**
	 * Override no longer pure method to make default implemntation visible.
	 */
	Observation get(environment::State const& state) override {
		PYBIND11_OVERLOAD(Observation, PyObsFunction, get, state);
	}
};

}  // namespace internal

/*************************
 *  User facing aliases  *
 *************************/

/**
 * Alias for Python observation abstract class.
 */
using ObsBase = internal::Py_ObsBase;

/**
 * Alias for Python observation class.
 *
 * Set the container type to @ref container::pytensor.
 *
 * @tparam An observation template class
 */
template <template <typename> class Observation>
using Obs = internal::Py_Obs<Observation<container::pytensor>>;

/**
 * Alias for Python observation function abstract class.
 */
using ObsFunctionBase = internal::Py_ObsFunctionBase;

/**
 * Alias for Python observation function class.
 *
 * Set the container type to @ref container::pytensor.
 *
 * @tparam An observation function template class
 */
template <template <typename> class ObservationFunction>
using ObsFunction = internal::Py_ObsFunction<ObservationFunction<container::pytensor>>;

/**
 * The @ref pybind11::class_ type for @ref ObsBase
 *
 * Set the holder type to @ref std::shared_ptr inheriting observation functions prevent
 * from having it as a @ref std::unique_ptr.
 */
using abstract_obs_class_ = py11::class_<
	ObsBase,                  // Class
	std::shared_ptr<ObsBase>  // Holder
	>;

/**
 * The @ref pybind11::class_ type for all observation.
 *
 * Set the parent abstract class.
 * Set the holder type to @ref std::shared_ptr inheriting observation functions prevent
 * from having it as a @ref std::unique_ptr.
 *
 * @tparam Observation The C++ observation to bind to Python
 */
template <template <typename> class Observation>
using obs_class_ = py11::class_<
	Obs<Observation>,                  // Class
	ObsBase,                           // Base
	std::shared_ptr<Obs<Observation>>  // Holder
	>;

/**
 * The @ref pybind11::class_ type for @ref observation::ObservationFunction.
 *
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to be stored in environments).
 */
using abstract_func_class_ = py11::class_<
	ObsFunctionBase,                                           // Class
	internal::Py_ObsFunctionBase_Trampoline<ObsFunctionBase>,  // Trampoline
	std::shared_ptr<ObsFunctionBase>                           // Holder
	>;

/**
 * The @ref pybind11::class_ type for all observation functions.
 *
 * Set the parent abstract class.
 * Set the trampoline class.
 * Set the holder type to @ref std::shared_ptr (as the objects created from Python needs
 * to be stored in environments).
 *
 * @tparam ObsSapce The C++ observation function to bind to Python
 */
template <template <typename> class ObservationFunction>
using function_class_ = py11::class_<
	ObsFunction<ObservationFunction>,                                       // Class
	ObsFunctionBase,                                                        // Base
	internal::Py_ObsFunction_Trampoline<ObsFunction<ObservationFunction>>,  // Trampoline
	std::shared_ptr<ObsFunction<ObservationFunction>>                       // Holder
	>;

}  // namespace pyobservation
}  // namespace ecole
