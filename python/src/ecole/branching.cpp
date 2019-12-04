#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/branching.hpp"
#include "ecole/observation.hpp"

#include "base.hpp"

namespace py11 = pybind11;
using namespace ecole;

PYBIND11_MODULE(branching, m) {
	m.doc() = "Learning to branch task.";
	// Import of base required for resolving inheritance to base types
	py11::module base_mod = py11::module::import("ecole.base");
	py11::module obs_mod = py11::module::import("ecole.observation");

	using ActionSpace = py::ActionSpaceBase<branching::ActionSpace>;
	using Fractional = py::ActionSpace<branching::Fractional, branching::ActionSpace>;
	using Env = py::Env<branching::Env>;

	py11::class_<ActionSpace>(m, "ActionSpace");
	py11::class_<Fractional, ActionSpace>(m, "Fractional")  //
		.def(py11::init<>());

	py11::class_<Env, py::EnvBase>(m, "Env")  //
		.def_static(
			"make_dummy",
			[] {
				return std::make_unique<Env>(
					std::make_unique<py::ObsSpace<obs::BasicObsSpace>>(),
					std::make_unique<Fractional>());
			})
		.def(
			py11::init([](py::ObsSpaceBase const& obs_space, ActionSpace const& action_space) {
				return std::make_unique<Env>(obs_space.clone(), action_space.clone());
			}))
		.def("step", [](py::EnvBase& env, branching::Fractional::action_t const& action) {
			return env.step(py::Action<std::size_t>(action));
		});
	;
}
