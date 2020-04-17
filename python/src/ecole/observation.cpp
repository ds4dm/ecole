#include <memory>
#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "ecole/utility/sparse_matrix.hpp"

namespace py = pybind11;
using namespace ecole;

template <typename ObservationFunction>
auto observation_function_class(py::module& m, char const* name) {
	return py::class_<ObservationFunction>(m, name)  //
		.def(py::init<>())
		.def("reset", &ObservationFunction::reset, py::arg("state"))
		.def("get", &ObservationFunction::get, py::arg("state"));
}

PYBIND11_MODULE(observation, m) {
	m.doc() = "Observation classes for ecole.";

	xt::import_numpy();

	using coo_matrix = decltype(observation::NodeBipartiteObs::matrix);
	py::class_<coo_matrix>(m, "coo_matrix")
		.def_property_readonly(
			"values", [](coo_matrix & self) -> auto& { return self.values; })
		.def_property_readonly(
			"indices", [](coo_matrix & self) -> auto& { return self.indices; })
		.def_property_readonly(
			"shape",
			[](coo_matrix& self) { return std::make_pair(self.shape[0], self.shape[1]); })
		.def_property_readonly("nnz", &coo_matrix::nnz);

	py::class_<observation::NodeBipartiteObs>(m, "NodeBipartiteObs")  //
		.def_property_readonly(
			"col_feat",
			[](observation::NodeBipartiteObs & self) -> auto& { return self.col_feat; })
		.def_property_readonly(
			"row_feat",
			[](observation::NodeBipartiteObs & self) -> auto& { return self.row_feat; })
		.def_readwrite("matrix", &observation::NodeBipartiteObs::matrix);

	observation_function_class<observation::NodeBipartite>(m, "NodeBipartite");
}
