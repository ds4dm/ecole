#include <memory>

#include <pybind11/pybind11.h>

#include "ecole/observation.hpp"

#include "base.hpp"
#include "base/observation.hpp"

namespace py11 = pybind11;
using namespace ecole;

PYBIND11_MODULE(observation, m) {
	m.doc() = "Observation classes for ecole.";

	auto const base_module = py11::module::import("ecole.base");

	py::obs::obs_class_<obs::BasicObs>(m, "BasicObs");

	py::obs::space_class_<obs::BasicObsSpace>(m, "BasicObsSpace")  //
		.def(py11::init<>());
}
