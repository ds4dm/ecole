#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

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

	using ActionFunction = py::ActionFunctionBase<branching::ActionFunction>;
	using Fractional = py::ActionFunction<branching::Fractional, branching::ActionFunction>;
	using Env = py::Env<branching::Env>;

	py11::class_<ActionFunction, std::shared_ptr<ActionFunction>>(m, "ActionFunction");
	py11::class_<Fractional, ActionFunction, std::shared_ptr<Fractional>>(
		m, "Fractional")  //
		.def(py11::init<>());

	py11::class_<Env, py::EnvBase>(m, "Env")  //
		.def_static(
			"make_dummy",
			[] {
				return std::make_unique<Env>(
					std::make_unique<Fractional>(),
					std::make_unique<py::obs::ObsFunction<obs::BasicObsFunction>>(),
					std::make_unique<reward::Done>(),
					std::make_unique<termination::Solved>());
			})
		.def(py11::init<
				 Env::ptr<ActionFunction> const&,
				 Env::ptr<py::obs::ObsFunctionBase> const&,
				 Env::ptr<base::RewardFunction> const&,
				 Env::ptr<base::TerminationFunction> const&>())  //
		.def("step", [](py::EnvBase& env, branching::Fractional::action_t const& action) {
			return env.step(py::Action<std::size_t>(action));
		});
	;
}
