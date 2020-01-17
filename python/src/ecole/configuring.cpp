#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#define FORCE_IMPORT_ARRAY
#include <xtensor-python/pytensor.hpp>

#include "ecole/configuring.hpp"
#include "ecole/observation.hpp"
#include "ecole/scip/model.hpp"

#include "base.hpp"

namespace py11 = pybind11;
using namespace ecole;

namespace ecole {
namespace configuring {

template <>
void Configure<py11::object>::set(scip::Model& model, action_t const& action) {
	if (py11::isinstance<py11::bool_>(action))
		model.set_param(param, action.cast<scip::param_t<scip::ParamType::Bool>>());
	else if (py11::isinstance<py11::int_>(action))
		// Casting to more precise, and may be downcasted in set_param call
		model.set_param(param, action.cast<scip::param_t<scip::ParamType::LongInt>>());
	else if (py11::isinstance<py11::float_>(action))
		model.set_param(param, action.cast<scip::param_t<scip::ParamType::Real>>());
	else if (py11::isinstance<py11::str>(action)) {
		// Cast as std::string and let set_param do conversion for char
		model.set_param(param, action.cast<std::string>().c_str());
	} else
		// Get exception from set_param
		model.set_param(param, action);
}

}  // namespace configuring
}  // namespace ecole

PYBIND11_MODULE(configuring, m) {
	m.doc() = "Learning to configure task.";
	// Import of base required for resolving inheritance to base types
	py11::module base_mod = py11::module::import("ecole.base");

	using ActionSpace = py::ActionSpaceBase<configuring::ActionSpace>;
	using Configure =
		py::ActionSpace<configuring::Configure<py11::object>, configuring::ActionSpace>;
	using Env = py::Env<configuring::Env>;

	py11::class_<ActionSpace, std::shared_ptr<ActionSpace>>(m, "ActionSpace");
	py11::class_<Configure, ActionSpace, std::shared_ptr<Configure>>(m, "Configure")  //
		.def(py11::init<std::string const&>())
		.def("set", [](Configure& c, scip::Model model, py11::object param) {
			c.set(model, py::Action<py11::object>(param));
		});

	py11::class_<Env, py::EnvBase>(m, "Env")  //
		.def_static(
			"make_dummy",
			[](std::string const& param) {
				return std::make_unique<Env>(
					std::make_unique<py::obs::ObsSpace<obs::BasicObsSpace>>(),
					std::make_unique<Configure>(param));
			})
		.def(py11::init(
			[](py::obs::ObsSpaceBase const& obs_space, ActionSpace const& action_space) {
				return std::make_unique<Env>(obs_space.clone(), action_space.clone());
			}))
		.def("step", [](py::EnvBase& env, py11::object const& action) {
			return env.step(py::Action<py11::object>(action));
		});
}
