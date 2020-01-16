#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include "ecole/scip/model.hpp"

namespace py11 = pybind11;
using namespace ecole;

// Specialize Model get_paramter method for dymaic return type
template <> py11::object scip::Model::get_param(const char* name) const {
	switch (get_param_type(name)) {
	case ParamType::Bool: {
		auto const& value = get_param_explicit<param_t<ParamType::Bool>>(name);
		// Scip bool type get converted to integer
		return py11::cast(static_cast<bool>(value));
	}
	case ParamType::Int:
		return py11::cast(get_param_explicit<param_t<ParamType::Int>>(name));
	case ParamType::LongInt:
		return py11::cast(get_param_explicit<param_t<ParamType::LongInt>>(name));
	case ParamType::Real:
		return py11::cast(get_param_explicit<param_t<ParamType::Real>>(name));
	case ParamType::Char:
		return py11::cast(get_param_explicit<param_t<ParamType::Char>>(name));
	case ParamType::String:
		return py11::cast(get_param_explicit<param_t<ParamType::String>>(name));
	default:
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception("Could not find type for given parameter");
	}
}

PYBIND11_MODULE(scip, m) {
	m.doc() = "Scip wrappers for ecole.";

	py11::class_<scip::Model>(m, "Model")  //
		.def_static("from_file", &scip::Model::from_file)

		.def(py11::self == py11::self)
		.def(py11::self != py11::self)

		.def(
			"get_param",
			[](scip::Model const& model, const char* name) {
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
