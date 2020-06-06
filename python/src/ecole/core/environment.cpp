#include <limits>
#include <memory>
#include <string>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/environment/branching-dynamics.hpp"
#include "ecole/environment/configuring-dynamics.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/scip/model.hpp"

#include "core.hpp"

namespace ecole {
namespace environment {

namespace py = pybind11;

template <typename Dynamics> auto dynamics_class(py::module& m, char const* name) {
	return py::class_<Dynamics>(m, name)  //
		.def(
			"reset_dynamics",
			&Dynamics::reset_dynamics,
			py::arg("model"),
			py::call_guard<py::gil_scoped_release>())
		.def(
			"step_dynamics",
			&Dynamics::step_dynamics,
			py::arg("model"),
			py::arg("action"),
			py::call_guard<py::gil_scoped_release>())
		.def(
			"set_dynamics_random_state",
			&Dynamics::set_dynamics_random_state,
			py::arg("model"),
			py::arg("random_engine"));
}

void bind_submodule(pybind11::module m) {
	m.doc() = "Ecole collection of environments.";

	py::register_exception<Exception>(m, "Exception");

	py::class_<RandomEngine>(m, "RandomEngine")  //
		.def_property_readonly_static(
			"min_seed",
			[](py::object /* cls */) {
				return std::numeric_limits<RandomEngine::result_type>::min();
			})
		.def_property_readonly_static(
			"max_seed",
			[](py::object /* cls */) {
				return std::numeric_limits<RandomEngine::result_type>::max();
			})
		.def(
			py::init<RandomEngine::result_type>(),
			py::arg("value") = RandomEngine::default_seed,
			"Construct the pseudo-random number engine.")
		.def(
			"seed",
			[](RandomEngine& self, RandomEngine::result_type value) { self.seed(value); },
			py::arg("value") = RandomEngine::default_seed,
			"Reinitialize the internal state of the random-number engine using new seed "
			"value.")
		.def(py::self == py::self)
		.def(py::self != py::self);

	dynamics_class<BranchingDynamics>(m, "BranchingDynamics")  //
		.def(py::init<bool>(), py::arg("pseudo_candidates") = false);

	dynamics_class<ConfiguringDynamics>(m, "ConfiguringDynamics")  //
		.def(py::init<>());
}

}  // namespace environment
}  // namespace ecole
