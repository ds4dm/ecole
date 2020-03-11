#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/abstract.hpp"
#include "ecole/scip/model.hpp"

#include "environment/adaptor.hpp"
#include "nonstd.hpp"
#include "observation/adaptor.hpp"
#include "reward/adaptor.hpp"
#include "termination/adaptor.hpp"

namespace py = pybind11;
using namespace ecole;

PYBIND11_MODULE(abstract, m) {
	m.doc() = "Abstract base classes for ecole environments.";

	auto const scip_module = py::module::import("ecole.scip");

	pyenvironment::base_state_class_(m, "State")  //
		.def(py::init<scip::Model const&>())
		.def_readwrite("model", &environment::State::model);

	pyobservation::abstract_func_class_(m, "ObservationFunction")
		.def(py::init<>())
		.def("reset", &pyobservation::ObsFunctionBase::reset, py::arg("state"))
		.def("get", &pyobservation::ObsFunctionBase::get, py::arg("state"));

	pyreward::abstract_func_class_(m, "RewardFunction")
		.def(py::init<>())
		.def("reset", &reward::RewardFunction::reset, py::arg("state"))
		.def("get", &reward::RewardFunction::get, py::arg("state"), py::arg("done") = false);

	pytermination::abstract_func_class_(m, "TerminationFunction")
		.def(py::init<>())
		.def("reset", &termination::TerminationFunction::reset, py::arg("state"))
		.def("is_done", &termination::TerminationFunction::is_done, py::arg("state"));

	pyenvironment::abstract_env_class_(m, "Environment")  //
		.def("seed", py::overload_cast<>(&pyenvironment::EnvBase::seed, py::const_))
		.def(
			"seed",
			py::overload_cast<environment::Seed>(&pyenvironment::EnvBase::seed),
			py::arg("value"))

		.def(
			"reset",
			py::overload_cast<std::string const&>(&pyenvironment::EnvBase::reset),
			py::arg("filename"))
		.def(
			"reset",
			py::overload_cast<scip::Model const&>(&pyenvironment::EnvBase::reset),
			py::arg("filename"))

		.def("step", &pyenvironment::EnvBase::step, py::arg("action"));
}
