#include <cstdint>
#include <memory>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include "ecole/scip/model.hpp"
#include "ecole/scip/scimpl.hpp"

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
		.def_static(
			"from_pyscipopt",
			[](py::object pyscipopt_model) {
				if (pyscipopt_model.attr("_freescip").cast<bool>()) {
					auto pyptr = pyscipopt_model.attr("to_ptr")(py::arg("give_ownership") = true);
					std::unique_ptr<SCIP, ScipDeleter> uptr = nullptr;
					uptr.reset(reinterpret_cast<SCIP*>(pyptr.cast<std::uintptr_t>()));
					return Model(std::make_unique<Scimpl>(std::move(uptr)));
				} else {
					throw scip::Exception(
						"Cannot create an Ecole Model from a non-owning PyScipOpt pointer.");
				}
			},
			// Keep the scip::Model (owner of the pointer) at least until the PyScipOpt model
			// is alive, as PyScipOpt is now sharing a non-owning pointer.
			py::keep_alive<1, 0>(),
			py::arg("model"))

		.def(py::self == py::self)
		.def(py::self != py::self)

		.def("copy_orig", &Model::copy_orig, py::call_guard<py::gil_scoped_release>())
		.def(
			"as_pyscipopt",
			[](scip::Model const& model) {
				auto const pyscipopt_module = py::module::import("pyscipopt.scip");
				auto const Model_class = pyscipopt_module.attr("Model");
				auto const ptr = reinterpret_cast<std::uintptr_t>(model.get_scip_ptr());
				return Model_class.attr("from_ptr")(ptr, py::arg("take_ownership") = false);
			},
			// Keep the scip::Model (owner of the pointer) at least until the PyScipOpt model
			// is alive, as PyScipOpt is a view on the ecole Model.
			py::keep_alive<0, 1>())

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
		.def("disable_presolve", &Model::disable_presolve)

		.def("solve", &Model::solve, py::call_guard<py::gil_scoped_release>());
}

}  // namespace scip
}  // namespace ecole
