#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include "ecole/branching.hpp"
#include "ecole/observation.hpp"

namespace py = pybind11;

using namespace ecole;

PYBIND11_MODULE(branching, m) {
	m.doc() = "Ecole library";

	py::class_<obs::BasicObs>(m, "BasicObs");

	using Env = branching::Env<obs::BasicObs, branching::Fractional::action_t>;
	py::class_<Env>(m, "Env") //
		.def(
			"make_default",
			[](std::string param) {
				return Env(
					std::make_unique<obs::BasicObsSpace>(),
					std::make_unique<branching::Fractional>());
			})
		.def("reset", (std::tuple<Env::obs_t, bool>(Env::*)(std::string)) & Env::reset)
		.def("step", &Env::step);
}
