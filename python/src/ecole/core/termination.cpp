#include <pybind11/pybind11.h>

#include "ecole/termination/constant.hpp"
#include "ecole/termination/whensolved.hpp"

#include "core.hpp"

namespace ecole {
namespace termination {

namespace py = pybind11;

template <typename TerminationFunction>
auto termination_function_class(py::module& m, char const* name) {
	return py::class_<TerminationFunction>(m, name)  //
		.def("reset", &TerminationFunction::reset, py::arg("state"))
		.def("is_done", &TerminationFunction::is_done, py::arg("state"));
}

void bind_submodule(py::module m) {
	m.doc() = "Termination classes for Ecole.";

	termination_function_class<Constant>(m, "Constant")  //
		.def(py::init<bool>(), py::arg("constant") = false);

	termination_function_class<WhenSolved>(m, "WhenSolved")  //
		.def(py::init<>());
}

}  // namespace termination
}  // namespace ecole
