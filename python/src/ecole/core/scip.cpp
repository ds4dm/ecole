#include <memory>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include "ecole/scip/model.hpp"

#include "core.hpp"

namespace ecole {
namespace scip {

namespace py = pybind11;

// Specialize Model get_paramter method for dynamic return type
template <> py::object Model::get_param(std::string const& name) const {
	switch (get_param_type(name)) {
	case ParamType::Bool:
		return py::cast(get_param_explicit<ParamType::Bool>(name));
	case ParamType::Int:
		return py::cast(get_param_explicit<ParamType::Int>(name));
	case ParamType::LongInt:
		return py::cast(get_param_explicit<ParamType::LongInt>(name));
	case ParamType::Real:
		return py::cast(get_param_explicit<ParamType::Real>(name));
	case ParamType::Char:
		return py::cast(get_param_explicit<ParamType::Char>(name));
	case ParamType::String:
		return py::cast(get_param_explicit<ParamType::String>(name));
	default:
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception("Could not find type for given parameter");
	}
}

void bind_submodule(py::module m) {
	m.doc() = "Scip wrappers for ecole.";

	py::register_exception<scip::Exception>(m, "Exception");

	py::class_<Model, std::shared_ptr<Model>>(m, "Model")  //
		.def_static("from_file", &Model::from_file)

		.def(py::self == py::self)
		.def(py::self != py::self)

		.def("clone", [](Model const& model) { return model; })

		.def(
			"get_param",
			[](Model const& model, std::string const& name) {
				// Use the variant to bind
				return model.get_param<py::object>(name);
			})
		.def(
			"set_param",
			[](Model& model, std::string const& name, py::bool_ value) {
				model.set_param(name, value);
			})
		.def(
			"set_param",
			[](Model& model, std::string const& name, py::int_ value) {
				model.set_param(name, value);
			})
		.def(
			"set_param",
			[](Model& model, std::string const& name, py::float_ value) {
				model.set_param(name, value);
			})
		.def(
			"set_param",
			[](Model& model, std::string const& name, std::string const& value) {
				model.set_param(name, value);
			})
		.def("disable_cuts", &Model::disable_cuts)
		.def("disable_presolve", &Model::disable_presolve);
}

}  // namespace scip
}  // namespace ecole
