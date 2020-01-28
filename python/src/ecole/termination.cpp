#include <pybind11/pybind11.h>

#include "ecole/termination/whensolved.hpp"

#include "wrapper/termination.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(termination, m) {
	m.doc() = "Termination classes for ecole.";

	auto const base_module = py11::module::import("ecole.base");

	pytermination::function_class_<termination::WhenSolved>(m, "WhenSolved")  //
		.def(py11::init<>());
}
