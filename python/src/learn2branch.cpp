#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include "ecole/env/learn2branch.hpp"
#include "ecole/env/learn2conf.hpp"
#include "ecole/env/observation.hpp"

namespace py = pybind11;

using namespace ecole;

PYBIND11_MODULE(ecole, m) {
	m.doc() = "Ecole library";

	py::class_<env::BasicObs>(m, "BasicObs");

	using Env = env::ConfEnv<env::BasicObs, bool>;
	py::class_<Env>(m, "ConfEnv") //
		.def(
			"make_default",
			[](std::string param) {
				return Env(
					std::make_unique<env::BasicObsSpace>(),
					std::make_unique<Env::Configure>(std::move(param)));
			})
		.def("reset", (std::tuple<Env::obs_t, bool>(Env::*)(std::string)) & Env::reset)
		.def("step", &Env::step);
}
