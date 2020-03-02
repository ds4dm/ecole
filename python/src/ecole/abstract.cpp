#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/abstract.hpp"
#include "ecole/scip/model.hpp"

#include "wrapper/environment.hpp"
#include "wrapper/observation.hpp"
#include "wrapper/reward.hpp"
#include "wrapper/termination.hpp"

namespace py = pybind11;
using namespace ecole;

PYBIND11_MODULE(abstract, m) {
	m.doc() = "Abstract base classes for ecole environments.";

	auto const scip_module = py::module::import("ecole.scip");

	pyobservation::abstract_func_class_(m, "ObservationFunction")
		.def(py::init<>())
		.def("reset", &pyobservation::ObsFunctionBase::reset, py::arg("model"))
		.def("get", &pyobservation::ObsFunctionBase::get, py::arg("model"));

	pyreward::abstract_func_class_(m, "RewardFunction")
		.def(py::init<>())
		.def("reset", &reward::RewardFunction::reset, py::arg("model"))
		.def("get", &reward::RewardFunction::get, py::arg("model"), py::arg("done") = false);

	pytermination::abstract_func_class_(m, "TerminationFunction")
		.def(py::init<>())
		.def("reset", &termination::TerminationFunction::reset, py::arg("model"))
		.def("is_done", &termination::TerminationFunction::is_done, py::arg("model"));

	pyenvironment::abstract_env_class_(m, "Environment")  //
		.def("seed", py::overload_cast<>(&pyenvironment::EnvBase::seed, py::const_))
		.def(
			"seed",
			py::overload_cast<environment::Seed>(&pyenvironment::EnvBase::seed),
			py::arg("value"))

		.def("reset", &pyenvironment::EnvBase::reset, py::arg("filename"))

		.def("step", &pyenvironment::EnvBase::step, py::arg("action"));
}
