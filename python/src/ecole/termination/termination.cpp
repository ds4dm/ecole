#include <pybind11/pybind11.h>

#include "ecole/termination/whensolved.hpp"

#include "termination/adaptor.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(termination, m) {
	m.doc() = "Termination classes for ecole.";

	auto const abstract_module = py11::module::import("ecole.abstract");

	pytermination::function_class_<termination::WhenSolved>(m, "WhenSolved")  //
		.def(py11::init<>());
}
