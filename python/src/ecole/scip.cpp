#include <pybind11/pybind11.h>

#include "ecole/scip/model.hpp"

namespace py11 = pybind11;
using namespace ecole;

PYBIND11_MODULE(scip, m) {
	m.doc() = "Scip wrappers for ecole.";

	py11::class_<scip::Model>(m, "Model") //
		.def_static("from_file", &scip::Model::from_file)
		.def("disable_cuts", &scip::Model::disable_cuts)
		.def("disable_presolve", &scip::Model::disable_presolve);
}
