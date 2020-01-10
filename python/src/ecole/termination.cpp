#include <pybind11/pybind11.h>

#include "ecole/termination.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(termination, m) {
	m.doc() = "Termination classes for ecole.";

	auto base_module = py11::module::import("ecole.base");

	py11::class_<termination::Solved, base::TerminationSpace>(m, "Solved")  //
		.def(py11::init<>());
}
