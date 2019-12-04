#include <pybind11/pybind11.h>

#include "ecole/observation.hpp"

#include "base.hpp"

namespace py11 = pybind11;
using namespace ecole;

namespace ecole {
namespace py {

using BasicObs = py::Obs<obs::BasicObs>;
using BasicObsSpace = py::ObsSpace<obs::BasicObsSpace>;

}  // namespace py
}  // namespace ecole

PYBIND11_MODULE(observation, m) {
	m.doc() = "Observation classes for ecole.";

	py11::class_<py::ObsBase>(m, "Observation");
	py11::class_<py::ObsSpaceBase>(m, "ObservationSpace")  //
		.def("get", &py::ObsSpaceBase::get);

	py11::class_<py::BasicObs, py::ObsBase>(m, "BasicObs");
	py11::class_<py::BasicObsSpace, py::ObsSpaceBase>(m, "BasicObsSpace")  //
		.def(py11::init<>());
}
