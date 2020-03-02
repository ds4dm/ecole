#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/environment/configuring.hpp"

#include "wrapper/environment.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(configuring, m) {
	m.doc() = "Learning to configure task.";

	// Import of abstract required for resolving inheritance to abstract types
	py11::module abstract_mod = py11::module::import("ecole.abstract");

	pyenvironment::env_class_<environment::Configuring>(m, "Configuring")  //
		.def(py11::init<
				 std::shared_ptr<pyobservation::ObsFunctionBase>,
				 std::shared_ptr<reward::RewardFunction>,
				 std::shared_ptr<termination::TerminationFunction>>());
}
