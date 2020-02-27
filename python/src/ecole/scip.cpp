#include <memory>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include "ecole/scip/model.hpp"

namespace py11 = pybind11;
using namespace ecole;

PYBIND11_MODULE(scip, m) {
	m.doc() = "Scip wrappers for ecole.";

	py11::class_<scip::Model, std::shared_ptr<scip::Model>>(m, "Model")  //
		.def_static("from_file", &scip::Model::from_file)

		.def(py11::self == py11::self)
		.def(py11::self != py11::self)

		.def("clone", [](scip::Model const& model) { return model; })

		.def(
			"get_param",
			[](scip::Model const& model, const char* name) {
				// Use the variant to bind
				return model.get_param<py11::object>(name);
			})
		.def(
			"set_param",
			[](scip::Model& model, const char* name, py11::bool_ value) {
				model.set_param(name, value);
			})
		.def(
			"set_param",
			[](scip::Model& model, const char* name, py11::int_ value) {
				model.set_param(name, value);
			})
		.def(
			"set_param",
			[](scip::Model& model, const char* name, py11::float_ value) {
				model.set_param(name, value);
			})
		.def(
			"set_param",
			[](scip::Model& model, const char* name, const char* value) {
				model.set_param(name, value);
			})
		.def("disable_cuts", &scip::Model::disable_cuts)
		.def("disable_presolve", &scip::Model::disable_presolve);
}
