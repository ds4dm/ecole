#include <pybind11/pybind11.h>

#include "ecole/termination/constant.hpp"

#include "core.hpp"

namespace ecole {
namespace termination {

namespace py = pybind11;

template <typename TerminationFunction>
auto termination_function_class(py::module& m, char const* name) {
	return py::class_<TerminationFunction>(m, name)  //
		.def("reset", &TerminationFunction::reset, py::arg("model"))
		.def(
			"obtain_termination", &TerminationFunction::obtain_termination, py::arg("state"));
}

void bind_submodule(py::module m) {
	m.doc() = "Termination classes for Ecole.";

	termination_function_class<Constant>(m, "Constant")  //
		.def(py::init<bool>(), py::arg("constant") = false);
}

}  // namespace termination
}  // namespace ecole
