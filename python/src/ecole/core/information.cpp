#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/information/nothing.hpp"
#include "ecole/scip/model.hpp"

#include "core.hpp"

namespace ecole::information {

namespace py = pybind11;

/**
 * Information module bindings definitions.
 */
void bind_submodule(py::module_ const& m) {
	m.doc() = "Inforation classes for Ecole.";

	py::class_<Nothing>(m, "Nothing")
		.def(py::init<>())
		.def("before_reset", &Nothing::before_reset, py::arg("model"), "Do nothing.")
		.def("extract", &Nothing::extract, py::arg("model"), py::arg("done"), "Return an empty dictionnary.");
}

}  // namespace ecole::information
