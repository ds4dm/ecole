#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/environment/branching.hpp"

#include "wrapper/environment.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(branching, m) {
	m.doc() = "Learning to branch task.";

	// Import of abstract required for resolving inheritance to abstract base types
	py11::module const abstract_mod = py11::module::import("ecole.abstract");

	pyenvironment::env_class_<environment::Branching>(m, "Configuring")  //
		.def(py11::init<
				 std::shared_ptr<pyobservation::ObsFunctionBase>,
				 std::shared_ptr<reward::RewardFunction>,
				 std::shared_ptr<termination::TerminationFunction>>());
}
