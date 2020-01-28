#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "ecole/base.hpp"
#include "ecole/scip/model.hpp"

#include "wrapper/environment.hpp"
#include "wrapper/observation.hpp"
#include "wrapper/reward.hpp"
#include "wrapper/termination.hpp"

namespace py = pybind11;
using namespace ecole;

PYBIND11_MODULE(base, m) {
	m.doc() = "Abstract base classes for ecole environments.";

	auto const scip_module = py::module::import("ecole.scip");

	pyobservation::base_obs_class_(m, "Observation")  //
		.def(py::init<>());

	pyobservation::base_func_class_(m, "ObservationFunction")
		.def(py::init<>())
		.def("reset", &pyobservation::ObsFunctionBase::reset, py::arg("model"))
		.def("get", &pyobservation::ObsFunctionBase::get, py::arg("model"));

	pyreward::base_func_class_(m, "RewardFunction")
		.def(py::init<>())
		.def("reset", &reward::RewardFunction::reset, py::arg("model"))
		.def("get", &reward::RewardFunction::get, py::arg("model"), py::arg("done") = false);

	pytermination::base_func_class_(m, "TerminationFunction")
		.def(py::init<>())
		.def("reset", &termination::TerminationFunction::reset, py::arg("model"))
		.def("is_done", &termination::TerminationFunction::is_done, py::arg("model"));

	py::class_<pyenvironment::EnvBase>(m, "Env")  //
		.def("seed", py::overload_cast<>(&pyenvironment::EnvBase::seed, py::const_))
		.def(
			"seed",
			py::overload_cast<pyenvironment::EnvBase::seed_t>(&pyenvironment::EnvBase::seed),
			py::arg("value"))

		.def(
			"reset",
			[](pyenvironment::EnvBase& env, pyenvironment::EnvBase::ptr<scip::Model> model) {
				return env.reset(std::move(model));
			},
			py::arg("model"))
		.def(
			"reset",
			py::overload_cast<std::string const&>(&pyenvironment::EnvBase::reset),
			py::arg("filename"))

		.def(
			"step",
			[](pyenvironment::EnvBase& env, py::object action) {
				return env.step(pyenvironment::Action<py::object>(action));
			},
			py::arg("action"));
}
