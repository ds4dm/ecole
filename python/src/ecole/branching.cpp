#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/base.hpp"
#include "ecole/branching.hpp"
#include "ecole/observation.hpp"
#include "ecole/reward.hpp"
#include "ecole/termination.hpp"

#include "base.hpp"

namespace py11 = pybind11;
using namespace ecole;

PYBIND11_MODULE(branching, m) {
	m.doc() = "Learning to branch task.";
	// Import of base required for resolving inheritance to base types
	py11::module base_mod = py11::module::import("ecole.base");

	using ActionSpace = py::ActionSpaceBase<branching::ActionSpace>;
	using Fractional = py::ActionSpace<branching::Fractional, branching::ActionSpace>;
	using Env = py::Env<branching::Env>;

	py11::class_<ActionSpace, std::shared_ptr<ActionSpace>>(m, "ActionSpace");
	py11::class_<Fractional, ActionSpace, std::shared_ptr<Fractional>>(m, "Fractional")  //
		.def(py11::init<>());

	py11::class_<Env, py::EnvBase>(m, "Env")  //
		.def_static(
			"make_dummy",
			[] {
				return std::make_unique<Env>(
					std::make_unique<Fractional>(),
					std::make_unique<py::obs::ObsSpace<obs::BasicObsSpace>>(),
					std::make_unique<reward::Done>(),
					std::make_unique<termination::Solved>());
			})
		.def(py11::init<
				 Env::ptr<ActionSpace> const&,
				 Env::ptr<py::obs::ObsSpaceBase> const&,
				 Env::ptr<base::RewardSpace> const&,
				 Env::ptr<base::TerminationSpace> const&>())  //
		.def("step", [](py::EnvBase& env, branching::Fractional::action_t const& action) {
			return env.step(py::Action<std::size_t>(action));
		});
	;
}
