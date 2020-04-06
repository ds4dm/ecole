#include <memory>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include "ecole/scip/model.hpp"

namespace py11 = pybind11;
using namespace ecole;

// Specialize Model get_paramter method for dynamic return type
template <> py11::object scip::Model::get_param(std::string const& name) const {
	switch (get_param_type(name)) {
	case ParamType::Bool:
		return py11::cast(get_param_explicit<ParamType::Bool>(name));
	case ParamType::Int:
		return py11::cast(get_param_explicit<ParamType::Int>(name));
	case ParamType::LongInt:
		return py11::cast(get_param_explicit<ParamType::LongInt>(name));
	case ParamType::Real:
		return py11::cast(get_param_explicit<ParamType::Real>(name));
	case ParamType::Char:
		return py11::cast(get_param_explicit<ParamType::Char>(name));
	case ParamType::String:
		return py11::cast(get_param_explicit<ParamType::String>(name));
	default:
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception("Could not find type for given parameter");
	}
}

PYBIND11_MODULE(scip, m) {
	m.doc() = "Scip wrappers for ecole.";

	py11::register_exception<scip::Exception>(m, "Exception");

	py11::class_<scip::Model, std::shared_ptr<scip::Model>>(m, "Model")  //
		.def_static("from_file", &scip::Model::from_file)

		.def(py11::self == py11::self)
		.def(py11::self != py11::self)

		.def("clone", [](scip::Model const& model) { return model; })

		.def(
			"get_param",
			[](scip::Model const& model, std::string const& name) {
				// Use the variant to bind
				return model.get_param<py11::object>(name);
			})
		.def(
			"set_param",
			[](scip::Model& model, std::string const& name, py11::bool_ value) {
				model.set_param(name, value);
			})
		.def(
			"set_param",
			[](scip::Model& model, std::string const& name, py11::int_ value) {
				model.set_param(name, value);
			})
		.def(
			"set_param",
			[](scip::Model& model, std::string const& name, py11::float_ value) {
				model.set_param(name, value);
			})
		.def(
			"set_param",
			[](scip::Model& model, std::string const& name, std::string const& value) {
				model.set_param(name, value);
			})
		.def("disable_cuts", &scip::Model::disable_cuts)
		.def("disable_presolve", &scip::Model::disable_presolve);
}
