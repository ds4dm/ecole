#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/environment/branching.hpp"
#include "ecole/environment/configuring.hpp"

#include "environment/adaptor.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(environment, m) {
	m.doc() = "Ecole collection of environments.";

	// Import of abstract required for resolving inheritance to abstract base types
	py11::module const abstract_mod = py11::module::import("ecole.abstract");

	pyenvironment::env_class_<environment::Branching>(m, "Branching")  //
		.def(py11::init<
				 std::shared_ptr<pyobservation::ObsFunctionBase>,
				 std::shared_ptr<reward::RewardFunction>,
				 std::shared_ptr<termination::TerminationFunction>>());

	pyenvironment::env_class_<environment::Configuring>(m, "Configuring")  //
		.def(py11::init<
				 std::shared_ptr<pyobservation::ObsFunctionBase>,
				 std::shared_ptr<reward::RewardFunction>,
				 std::shared_ptr<termination::TerminationFunction>>());
}
