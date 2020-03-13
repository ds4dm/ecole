#include <memory>
#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "ecole/observation/none.hpp"
#include "ecole/utility/sparse_matrix.hpp"

#include "observation/adaptor.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(observation, m) {
	m.doc() = "Observation classes for ecole.";

	xt::import_numpy();

	auto const abstract_module = py11::module::import("ecole.abstract");

	using coo_matrix = decltype(observation::NodeBipartiteObs::matrix);
	py11::class_<coo_matrix>(m, "coo_matrix")
		.def_property_readonly(
			"values", [](coo_matrix & self) -> auto& { return self.values; })
		.def_property_readonly(
			"indices", [](coo_matrix & self) -> auto& { return self.indices; })
		.def_property_readonly(
			"shape",
			[](coo_matrix& self) -> std::pair<std::size_t, std::size_t> { return self.shape; })
		.def_property_readonly("nnz", &coo_matrix::nnz);

	pyobservation::obs_class_<observation::NodeBipartiteObs>(m, "NodeBipartiteObs")  //
		.def_property_readonly(
			"col_feat",
			[](observation::NodeBipartiteObs & self) -> auto& { return self.col_feat; })
		.def_property_readonly(
			"row_feat",
			[](observation::NodeBipartiteObs & self) -> auto& { return self.row_feat; })
		.def_readwrite("matrix", &observation::NodeBipartiteObs::matrix);

	pyobservation::function_class_<observation::NodeBipartite>(m, "NodeBipartite")
		.def(py11::init<>());

	pyobservation::obs_class_<observation::NoneObs>(m, "NoneObs");

	pyobservation::function_class_<observation::None>(m, "None_")  //
		.def(py11::init<>());
}
