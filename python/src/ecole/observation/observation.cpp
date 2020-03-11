#include <memory>

#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "ecole/observation/none.hpp"

#include "observation/adaptor.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(observation, m) {
	m.doc() = "Observation classes for ecole.";

	xt::import_numpy();

	auto const abstract_module = py11::module::import("ecole.abstract");

	pyobservation::obs_class_<observation::NodeBipartiteObs>(m, "NodeBipartiteObs")  //
		.def_property_readonly(
			"col_feat",
			[](observation::NodeBipartiteObs & self) -> auto& { return self.col_feat; });

	pyobservation::function_class_<observation::NodeBipartite>(m, "NodeBipartite")
		.def(py11::init<>());

	pyobservation::obs_class_<observation::NoneObs>(m, "NoneObs");

	pyobservation::function_class_<observation::None>(m, "None_")  //
		.def(py11::init<>());
}
