#include <memory>

#include <fmt/format.h>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include "ecole/scip/model.hpp"

namespace py11 = pybind11;
using namespace ecole;
using namespace scip;

PYBIND11_MODULE(scip, m) {
	m.doc() = "Scip wrappers for ecole.";

	py11::register_exception<Exception>(m, "Exception");

	py11::class_<Model, std::shared_ptr<Model>>(m, "Model")  //
		.def_static("from_file", &Model::from_file)

		.def(py11::self == py11::self)
		.def(py11::self != py11::self)

		.def("clone", [](Model const& model) { return model; })

		.def(
			"get_param",
			[](Model const& model, std::string const& name) {
				// handle dynamic python type here
				switch (model.get_param_type(name)) {
				case ParamType::Bool:
					return py11::cast(model.get_param_explicit<ParamType::Bool>(name));
				case ParamType::Int:
					return py11::cast(model.get_param_explicit<ParamType::Int>(name));
				case ParamType::LongInt:
					return py11::cast(model.get_param_explicit<ParamType::LongInt>(name));
				case ParamType::Real:
					return py11::cast(model.get_param_explicit<ParamType::Real>(name));
				case ParamType::Char:
					return py11::cast(model.get_param_explicit<ParamType::Char>(name));
				case ParamType::String:
					return py11::cast(model.get_param_explicit<ParamType::String>(name));
				default:
					assert(false);  // All enum value should be handled
					// Non void return for optimized build
					throw Exception("Could not find type for given parameter");
				}
			})
		// Char and Int types are not necessary, proper conversion will happen later on anyway
		// Char type actually confuses pybind11 with string arguments
		// https://pybind11.readthedocs.io/en/stable/advanced/functions.html#overload-resolution-order
		.def(
			"set_param",
			[](Model& model, std::string const& name, param_t<ParamType::Bool> value) {
				model.set_param(name, value);
			},
			py11::arg(),
			py11::arg("value").noconvert()  // forbid None
			)
		.def(
			"set_param",
			[](Model& model, std::string const& name, param_t<ParamType::LongInt> value) {
				model.set_param(name, value);
			},
			py11::arg(),
			py11::arg("value").noconvert()  // forbid None
			)
		.def(
			"set_param",
			[](Model& model, std::string const& name, param_t<ParamType::Real> value) {
				model.set_param(name, value);
			},
			py11::arg(),
			py11::arg("value").noconvert()  // forbid None
			)
		.def(
			"set_param",
			[](Model& model, std::string const& name, param_t<ParamType::String>& value) {
				model.set_param(name, value);
			},
			py11::arg(),
			py11::arg("value").noconvert()  // forbid None
			)
		.def("disable_cuts", &Model::disable_cuts)
		.def("disable_presolve", &Model::disable_presolve);
}
