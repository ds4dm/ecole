#include <pybind11/pybind11.h>

#include "ecole/termination.hpp"

#include "base/termination.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(termination, m) {
	m.doc() = "Termination classes for ecole.";

	auto const base_module = py11::module::import("ecole.base");

	py::termination::function_class_<termination::Solved>(m, "Solved")  //
		.def(py11::init<>());
}
