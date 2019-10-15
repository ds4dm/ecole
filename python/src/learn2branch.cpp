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
		.def_static(
			"make_default",
			[](std::string const& filename) {
				auto model = scip::Model::from_file(filename);
				model.disable_cuts();
				model.disable_presolve();
				return BranchEnv{std::move(model), std::make_unique<BasicObs::Factory>()};
			})
		.def("run", &BranchEnv::run);
}
