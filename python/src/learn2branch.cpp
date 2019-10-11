#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include "ecole/learn2branch.hpp"
#include "ecole/observation.hpp"

namespace py = pybind11;

using namespace ecole;

PYBIND11_MODULE(ecole, m) {
	m.doc() = "Ecole library";

	py::class_<Observation>(m, "Observation");

	py::class_<BranchEnv>(m, "BranchEnv") //
		.def_static("from_file", &BranchEnv::from_file)
		.def("disable_presolve", [](BranchEnv& env) { env.model.disable_presolve(); })
		.def("disable_cuts", [](BranchEnv& env) { env.model.disable_cuts(); })
		.def("run", &BranchEnv::run);
}
