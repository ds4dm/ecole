#include <memory>
#include <utility>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/observation/nodebipartite.hpp"
#include "ecole/observation/nothing.hpp"
#include "ecole/utility/sparse_matrix.hpp"

#include "core.hpp"
#include "nonstd.hpp"

namespace ecole {
namespace observation {

namespace py = pybind11;

template <typename ObservationFunction>
auto observation_function_class(py::module& m, char const* name) {
	return py::class_<ObservationFunction>(m, name)  //
		.def(
			"reset",
			&ObservationFunction::reset,
			py::arg("state"),
			py::call_guard<py::gil_scoped_release>())
		.def(
			"obtain_observation",
			&ObservationFunction::obtain_observation,
			py::arg("state"),
			py::call_guard<py::gil_scoped_release>());
}

void bind_submodule(py::module m) {
	m.doc() = "Observation classes for Ecole.";

	xt::import_numpy();

	observation_function_class<Nothing>(m, "Nothing")  //
		.def(py::init<>());

	using coo_matrix = decltype(NodeBipartiteObs::matrix);
	py::class_<coo_matrix>(m, "coo_matrix")
		.def_property_readonly(
			"values", [](coo_matrix & self) -> auto& { return self.values; })
		.def_property_readonly(
			"indices", [](coo_matrix & self) -> auto& { return self.indices; })
		.def_property_readonly(
			"shape",
			[](coo_matrix& self) { return std::make_pair(self.shape[0], self.shape[1]); })
		.def_property_readonly("nnz", &coo_matrix::nnz);

	py::class_<NodeBipartiteObs>(m, "NodeBipartiteObs")  //
		.def_property_readonly(
			"col_feat", [](NodeBipartiteObs & self) -> auto& { return self.col_feat; })
		.def_property_readonly(
			"row_feat", [](NodeBipartiteObs & self) -> auto& { return self.row_feat; })
		.def_readwrite("matrix", &NodeBipartiteObs::matrix);

	observation_function_class<NodeBipartite>(m, "NodeBipartite")  //
		.def(py::init<>());
}

}  // namespace observation
}  // namespace ecole
