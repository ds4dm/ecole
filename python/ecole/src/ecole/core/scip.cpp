#include <memory>

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl/filesystem.h>

#include "ecole/python/auto-class.hpp"
#include "ecole/scip/callback.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/scimpl.hpp"

#include "core.hpp"

namespace ecole::scip {

namespace py = pybind11;

namespace callback {

void bind_submodule(py::module_ m) {
	m.doc() = "Callback utilities for iterative solving.";

	py::enum_<Type>{m, "Type"}
		.value("Branchrule", Type::Branchrule)  //
		.value("Heuristic", Type::Heuristic);

	m.def("name", name, "Return the name used by the reverse callback.");

	m.attr("priority_max") = priority_max;
	m.attr("maxdepth_none") = max_depth_none;
	m.attr("maxbounddist_none") = max_bound_distance_none;
	m.attr("frequency_always") = frequency_always;
	m.attr("frequency_offset_none") = frequency_offset_none;

	python::auto_data_class<BranchruleConstructor>(m, "BranchruleConstructor")
		.def_auto_members(
			python::Member{"priority", &BranchruleConstructor::priority},
			python::Member{"max_depth", &BranchruleConstructor::max_depth},
			python::Member{"max_bound_distance", &BranchruleConstructor::max_bound_distance});

	python::auto_data_class<HeuristicConstructor>(m, "HeuristicConstructor")
		.def_auto_members(
			python::Member{"priority", &HeuristicConstructor::priority},
			python::Member{"frequency", &HeuristicConstructor::frequency},
			python::Member{"frequency_offset", &HeuristicConstructor::frequency_offset},
			python::Member{"max_depth", &HeuristicConstructor::max_depth},
			python::Member{"timing_mask", &HeuristicConstructor::timing_mask});

	auto branchrule_call = python::auto_data_class<BranchruleCall>(m, "Branchrulecall");
	py::enum_<BranchruleCall::Where>(branchrule_call, "Where")
		.value("LP", BranchruleCall::Where::LP)
		.value("External", BranchruleCall::Where::External)
		.value("Pseudo", BranchruleCall::Where::Pseudo);
	branchrule_call.def_auto_members(
		python::Member{"allow_add_constraints", &BranchruleCall::allow_add_constraints},
		python::Member{"where", &BranchruleCall::where});

	python::auto_data_class<HeuristicCall>(m, "HeuristicCall")
		.def_auto_members(
			python::Member{"heuristic_timing", &HeuristicCall::heuristic_timing},
			python::Member{"node_infeasible", &HeuristicCall::node_infeasible});
}

}  // namespace callback

void bind_submodule(py::module_ m) {
	m.doc() = "Scip wrappers for ecole.";

	py::register_exception<scip::ScipError>(m, "ScipError");

	py::enum_<SCIP_STAGE>{m, "Stage"}
		.value("Init", SCIP_STAGE_INIT)
		.value("Problem", SCIP_STAGE_PROBLEM)
		.value("Transforming", SCIP_STAGE_TRANSFORMING)
		.value("Transformed", SCIP_STAGE_TRANSFORMED)
		.value("InitPresolve", SCIP_STAGE_INITPRESOLVE)
		.value("Presolving", SCIP_STAGE_PRESOLVING)
		.value("ExitPresolve", SCIP_STAGE_EXITPRESOLVE)
		.value("Presolved", SCIP_STAGE_PRESOLVED)
		.value("InitSolve", SCIP_STAGE_INITSOLVE)
		.value("Solving", SCIP_STAGE_SOLVING)
		.value("Solved", SCIP_STAGE_SOLVED)
		.value("ExitSolve", SCIP_STAGE_EXITSOLVE)
		.value("FreeTrans", SCIP_STAGE_FREETRANS)
		.value("Free", SCIP_STAGE_FREE);

	// SCIP_HEURTIMING is simply a collection of Macros! We create a scope for holding the values.
	struct HeurTiming {};
	py::class_<HeurTiming>{m, "HeurTiming"}
		.def_property_readonly_static("DuringLpLoop", [](py::handle /*cls*/) { return SCIP_HEURTIMING_DURINGLPLOOP; })
		.def_property_readonly_static("AfterLpLoop", [](py::handle /*cls*/) { return SCIP_HEURTIMING_AFTERLPLOOP; })
		.def_property_readonly_static("AfterLpNode", [](py::handle /*cls*/) { return SCIP_HEURTIMING_AFTERLPNODE; })
		.def_property_readonly_static("AfterPseudoNode", [](py::handle /*cls*/) { return SCIP_HEURTIMING_AFTERPSEUDONODE; })
		.def_property_readonly_static("AfterLpPlunge", [](py::handle /*cls*/) { return SCIP_HEURTIMING_AFTERLPPLUNGE; })
		.def_property_readonly_static(
			"AfterPseudoPlunge", [](py::handle /*cls*/) { return SCIP_HEURTIMING_AFTERPSEUDOPLUNGE; })
		.def_property_readonly_static(
			"DuringPricingLoop", [](py::handle /*cls*/) { return SCIP_HEURTIMING_DURINGPRICINGLOOP; })
		.def_property_readonly_static("BeforePresol", [](py::handle /*cls*/) { return SCIP_HEURTIMING_BEFOREPRESOL; })
		.def_property_readonly_static(
			"DuringPresolLoop", [](py::handle /*cls*/) { return SCIP_HEURTIMING_DURINGPRESOLLOOP; })
		.def_property_readonly_static("AfterPropLoop", [](py::handle /*cls*/) { return SCIP_HEURTIMING_AFTERPROPLOOP; })
		.def_property_readonly_static("AfterNode", [](py::handle /*cls*/) { return SCIP_HEURTIMING_AFTERNODE; })
		.def_property_readonly_static("AfterPlunge", [](py::handle /*cls*/) { return SCIP_HEURTIMING_AFTERPLUNGE; });

	callback::bind_submodule(m.def_submodule("callback"));

	py::class_<Model>(m, "Model")  //
		.def_static("from_file", &Model::from_file, py::arg("filepath"), py::call_guard<py::gil_scoped_release>())
		.def_static("prob_basic", &Model::prob_basic, py::arg("name") = "Model")
		.def_static(
			"from_pyscipopt",
			[](py::object const& pyscipopt_model) {
				if (pyscipopt_model.attr("_freescip").cast<bool>()) {
					py::capsule cap = pyscipopt_model.attr("to_ptr")(py::arg("give_ownership") = true);
					std::unique_ptr<SCIP, ScipDeleter> uptr = nullptr;
					uptr.reset(reinterpret_cast<SCIP*>(py::cast<void*>(cap)));
					return Model{std::make_unique<Scimpl>(std::move(uptr))};
				}
				throw scip::ScipError{"Cannot create an Ecole Model from a non-owning PyScipOpt pointer."};
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

		.def("set_messagehdlr_quiet", &Model::set_messagehdlr_quiet, py::arg("quiet"))

		.def_property("name", &Model::name, &Model::set_name)
		.def_property_readonly("stage", &Model::stage)

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

		.def_property_readonly("is_solved", &Model::is_solved)
		.def_property_readonly("primal_bound", &Model::primal_bound)
		.def_property_readonly("dual_bound", &Model::dual_bound);
}

}  // namespace ecole::scip
