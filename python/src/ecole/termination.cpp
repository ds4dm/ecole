#include <pybind11/pybind11.h>

#include "ecole/termination/whensolved.hpp"

namespace py = pybind11;
using namespace ecole;

template <typename TerminationFunction>
auto termination_function_class(py::module& m, char const* name) {
	return py::class_<TerminationFunction>(m, name)  //
		.def(py::init<>())
		.def("reset", &TerminationFunction::reset, py::arg("state"))
		.def("is_done", &TerminationFunction::is_done, py::arg("state"));
}

PYBIND11_MODULE(termination, m) {
	m.doc() = "Termination classes for ecole.";

	termination_function_class<termination::WhenSolved>(m, "WhenSolved");
}
