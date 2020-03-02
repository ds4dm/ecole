#include <memory>

#include <pybind11/pybind11.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/observation/node-bipartite.hpp"

#include "observation/adaptor.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(observation, m) {
	m.doc() = "Observation classes for ecole.";

	xt::import_numpy();

	auto const abstract_module = py11::module::import("ecole.abstract");

	pyobservation::obs_class_<observation::NodeBipartiteObs> node_bipartite_binding(
		m, "NodeBipartiteObs");
	using py_node_bipartite_t = decltype(node_bipartite_binding)::type;
	node_bipartite_binding  //
		.def_property_readonly(
			"var_feat", [](py_node_bipartite_t const& obs) -> auto& { return obs.var_feat; });

	pyobservation::function_class_<observation::NodeBipartite>(m, "NodeBipartite")
		.def(py11::init<>());
}
