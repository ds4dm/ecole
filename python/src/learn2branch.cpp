#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include "ecole/env/learn2branch.hpp"
#include "ecole/env/observation.hpp"

namespace py = pybind11;

using namespace ecole;

PYBIND11_MODULE(ecole, m) {
	m.doc() = "Ecole library";

	py::class_<env::Observation>(m, "Observation");

	py::class_<env::BranchEnv>(m, "BranchEnv") //
		.def_static(
			"make_default",
			[](std::string const& filename) {
				auto model = scip::Model::from_file(filename);
				model.disable_cuts();
				model.disable_presolve();
				return env::BranchEnv{std::move(model),
															std::make_unique<env::BasicObs::Factory>()};
			})
		.def("run", &env::BranchEnv::run);
}
