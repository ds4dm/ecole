#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/branching.hpp"

#include "observation.hpp"

namespace py11 = pybind11;
using namespace ecole;

PYBIND11_MODULE(branching, m) {
	m.doc() = "Learning to branch task.";

	using obs_t = std::unique_ptr<py::ObsBase>;
	using action_t = branching::Fractional::action_t;
	using Env = branching::Env<obs_t, action_t>;
	py11::class_<Env>(m, "Env") //
		.def_static(
			"make_dummy",
			[] {
				return Env{std::make_unique<py::BasicObsSpace>(),
									 std::make_unique<branching::Fractional>()};
			})
		.def(py11::init([](py::ObsSpaceBase const& obs_space) {
			return std::make_unique<Env>(
				obs_space.clone(), std::make_unique<branching::Fractional>());
		}))
		.def("reset", py11::overload_cast<scip::Model>(&Env::reset))
		.def("reset", py11::overload_cast<std::string>(&Env::reset))
		.def("step", &Env::step);
}
