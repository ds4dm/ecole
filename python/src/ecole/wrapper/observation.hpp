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
 * Base class for all observation functions.
 *
 * All observation functions must inherit from this class before being bound to Python in
 * order to be properly passed to environments.
 */
using Py_ObsFunctionBase = observation::ObservationFunction<py11::object>;

/**
 * Adaptor to make C++ observation functions.
 *
 * Make the adapted class properly inherit @ref Py_ObsFunctionBase and return
 * observation in the proper way (using @refpy11::object)).
 *
 * @tparam ObsFunction The C++ observation function to adapt.
 */
template <typename ObsFunction> struct Py_ObsFunction : public Py_ObsFunctionBase {
	using typename Py_ObsFunctionBase::Observation;

	// TODO make a alias of State to be able to combine by inheritance (no return conflict).
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
	 * Cast the observation from the adapted @ref obs_func into a @ref py11::object.
	 */
	Observation get(environment::State const& state) override {
		return py11::cast(obs_func.get(state));
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
 * PyBind trampoline class for Python inheritance of vanilla @ref Py_ObsFunction classes.
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
 * Alias for Python observation function abstract class.
 */
using ObsFunctionBase = internal::Py_ObsFunctionBase;

/**
 * Alias for Python observation function class.
 *
 * Set the container type to @ref container::pytensor.
 *
 * @tparam ObservationFunction An observation function template class
 */
template <template <typename> class ObservationFunction>
using ObsFunction = internal::Py_ObsFunction<ObservationFunction<container::pytensor>>;

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
using obs_class_ = py11::class_<Observation<container::pytensor>>;

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
