#include <memory>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>

#include "ecole/scip/model.hpp"
#include "ecole/scip/scimpl.hpp"

#include "core.hpp"

namespace ecole::scip {

namespace py = pybind11;

void bind_submodule(py::module_ const& m) {
	m.doc() = "Scip wrappers for ecole.";

	py::register_exception<scip::Exception>(m, "Exception");

	py::class_<Model, std::shared_ptr<Model>>(m, "Model")  //
		.def_static("from_file", &Model::from_file, py::arg("filepath"), py::call_guard<py::gil_scoped_release>())
		.def_static("prob_basic", &Model::prob_basic, py::arg("name") = "Model")
		.def_static(
			"from_pyscipopt",
			[](py::object const& pyscipopt_model) {
				if (pyscipopt_model.attr("_freescip").cast<bool>()) {
					py::capsule cap = pyscipopt_model.attr("to_ptr")(py::arg("give_ownership") = true);
					std::unique_ptr<SCIP, ScipDeleter> uptr = nullptr;
					uptr.reset(reinterpret_cast<SCIP*>(py::cast<void*>(cap)));
					return Model(std::make_unique<Scimpl>(std::move(uptr)));
				}
				throw scip::Exception("Cannot create an Ecole Model from a non-owning PyScipOpt pointer.");
			},
			// Keep the scip::Model (owner of the pointer) at least until the PyScipOpt model
			// is alive, as PyScipOpt is now sharing a non-owning pointer.
			py::keep_alive<1, 0>(),
			py::arg("model"))

		.def(py::self == py::self)  // NOLINT(misc-redundant-expression)  pybind specific syntax
		.def(py::self != py::self)  // NOLINT(misc-redundant-expression)  pybind specific syntax

		.def("copy_orig", &Model::copy_orig, py::call_guard<py::gil_scoped_release>())
		.def(
			"as_pyscipopt",
			[](scip::Model& model) {
				auto const Model_class = py::module_::import("pyscipopt.scip").attr("Model");
				auto const cap = py::capsule{reinterpret_cast<void*>(model.get_scip_ptr()), "scip"};
				return Model_class.attr("from_ptr")(cap, py::arg("take_ownership") = false);
			},
			// Keep the scip::Model (owner of the pointer) at least until the PyScipOpt model
			// is alive, as PyScipOpt is a view on the ecole Model.
			py::keep_alive<0, 1>())

		.def("get_param", &Model::get_param<Param>, py::arg("name"))
		.def("set_param", &Model::set_param<Param>, py::arg("name"), py::arg("value"))
		.def("get_params", &Model::get_params)
		.def("set_params", &Model::set_params, py::arg("name_values"))
		.def("disable_cuts", &Model::disable_cuts)
		.def("disable_presolve", &Model::disable_presolve)
		.def("write_problem", &Model::write_problem, py::arg("filepath"), py::call_guard<py::gil_scoped_release>())

		.def("transform_prob", &Model::transform_prob, py::call_guard<py::gil_scoped_release>())
		.def("presolve", &Model::presolve, py::call_guard<py::gil_scoped_release>())
		.def("solve", &Model::solve, py::call_guard<py::gil_scoped_release>())
		.def("is_solved", &Model::is_solved);
}

}  // namespace ecole::scip
