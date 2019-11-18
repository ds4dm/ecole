#include <pybind11/pybind11.h>

#include "observation.hpp"

namespace py11 = pybind11;
using namespace ecole;

PYBIND11_MODULE(observation, m) {
	m.doc() = "Observation classes for ecole.";

	py11::class_<py::ObsBase>(m, "Observation");
	py11::class_<py::ObsSpaceBase>(m, "ObservationSpace") //
		.def("get", &py::ObsSpaceBase::get);

	py11::class_<py::BasicObs, py::ObsBase>(m, "BasicObs");
	py11::class_<py::BasicObsSpace, py::ObsSpaceBase>(m, "BasicObsSpace") //
		.def(py11::init<>());
}
