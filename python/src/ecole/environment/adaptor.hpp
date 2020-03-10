#pragma once

#include <memory>

#include <nonstd/optional.hpp>
#include <pybind11/pybind11.h>

#include "ecole/environment/abstract.hpp"
#include "ecole/environment/state.hpp"
#include "ecole/reward/abstract.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/termination/abstract.hpp"

#include "observation/adaptor.hpp"

namespace py11 = pybind11;

namespace ecole {
namespace pyenvironment {

using environment::Info;
using environment::Reward;
using environment::Seed;

/*********************************
 *  Base and trampoline classes  *
 *********************************/

namespace internal {

using nonstd::optional;

/**
 * Base class for all environments.
 *
 * All environments inherit from this class before being bound to Python in order to
 * properly recieve actions.
 */
using Py_EnvBase = environment::Environment<py11::object, py11::object>;

/**
 * Adaptor to make C++ environments.
 *
 * Inherits from the environment class and the @ref Py_EnvBase to combine methods in a way
 * that properly expose Python objects.
 *
 * @tparam Env The C++ Environment to adapt.
 */
template <typename Env> struct Py_Env : Py_EnvBase, Env {
	using Action = Py_EnvBase::Action;
	using Observation = Py_EnvBase::Observation;

	using Env::Env;

	using Env::state;

	/**
	 * @copydoc Py_EnvBase::seed
	 */
	void seed(Seed seed) override { return Env::seed(seed); }
	Seed seed() const noexcept override { return Env::seed(); }

	/**
	 * @copydoc Py_EnvBase::reset
	 */
	std::tuple<optional<Observation>, bool> reset(scip::Model&& model) override {
		return Env::reset(std::move(model));
	}
	std::tuple<optional<Observation>, bool> reset(scip::Model const& model) override {
		return Env::reset(model);
	}
	std::tuple<optional<Observation>, bool> reset(std::string const& filename) override {
		return Env::reset(filename);
	}

	/**
	 * Cast the action from the @ref py11::object into the adapted @ref Env.
	 */
	std::tuple<optional<Observation>, Reward, bool, Info>
	step(Action const& action) override {
		return Env::step(py11::cast<typename Env::Action>(action));
	}
};

}  // namespace internal

/*************************
 *  User facing aliases  *
 *************************/

/**
 * Alias for the State class.
 */
using base_state_class_ = py11::class_<
	environment::State,                  // Class
	std::shared_ptr<environment::State>  // Holder
	>;

/**
 * Alias for Python environment abstract class.
 */
using EnvBase = internal::Py_EnvBase;

/**
 * Alias for Python environment class.
 *
 * Set the state functions to a @ref std::shared_ptr of their binded type.
 *
 * @tparam Environment An environment template class
 */
template <template <typename...> class Environment>
using Env = internal::Py_Env<Environment<
	std::shared_ptr<pyobservation::ObsFunctionBase>,
	std::shared_ptr<reward::RewardFunction>,
	std::shared_ptr<termination::TerminationFunction>>>;

/**
 * The @ref pybind11::class_ type for @ref environment::Environment.
 */
using abstract_env_class_ = py11::class_<EnvBase>;

/**
 * The @ref pybind11::class_ type for all environment functions.
 *
 * Set the parent abstract class.
 *
 * @tparam Environment The C++ environment to bind to Python
 */
template <template <typename...> class Environment>
using env_class_ = py11::class_<  //
	Env<Environment>,               // Class
	EnvBase                         // Base
	>;

}  // namespace pyenvironment
}  // namespace ecole
