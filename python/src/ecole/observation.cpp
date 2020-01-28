#include <memory>

#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/observation/basicobs.hpp"

#include "wrapper/observation.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(observation, m) {
	m.doc() = "Observation classes for ecole.";

	xt::import_numpy();

	auto const base_module = py11::module::import("ecole.base");

	pyobservation::obs_class_<observation::BasicObs> basic_obs_bind(m, "BasicObs");
	using py_basic_obs_t = decltype(basic_obs_bind)::type;
	basic_obs_bind  //
		.def_property_readonly(
			"var_feat", [](py_basic_obs_t const& obs) -> auto& { return obs.obs.var_feat; });

	pyobservation::function_class_<observation::BasicObsFunction>(m, "BasicObsFunction")
		.def(py11::init<>());
}
