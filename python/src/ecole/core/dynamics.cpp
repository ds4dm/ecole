#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/dynamics/branching.hpp"
#include "ecole/dynamics/configuring.hpp"
#include "ecole/scip/model.hpp"

#include "core.hpp"

namespace ecole::dynamics {

namespace py = pybind11;

template <typename Dynamics> auto dynamics_class(py::module_ const& m, char const* name) {
	return py::class_<Dynamics>(m, name)  //
		.def("reset_dynamics", &Dynamics::reset_dynamics, py::arg("model"), py::call_guard<py::gil_scoped_release>())
		.def(
			"step_dynamics",
			&Dynamics::step_dynamics,
			py::arg("model"),
			py::arg("action"),
			py::call_guard<py::gil_scoped_release>())
		.def("set_dynamics_random_state", &Dynamics::set_dynamics_random_state, py::arg("model"), py::arg("random_engine"));
}

void bind_submodule(pybind11::module_ const& m) {
	m.doc() = "Ecole collection of environment dynamics.";

	dynamics_class<BranchingDynamics>(m, "BranchingDynamics")  //
		.def(py::init<bool>(), py::arg("pseudo_candidates") = false);

	dynamics_class<ConfiguringDynamics>(m, "ConfiguringDynamics")  //
		.def(py::init<>());
}

}  // namespace ecole::dynamics
