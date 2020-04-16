#include <pybind11/pybind11.h>

#include "ecole/reward/isdone.hpp"
#include "ecole/reward/neglpiterations.hpp"

#include "reward/adaptor.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(reward, m) {
	m.doc() = "Reward classes for ecole.";

	auto const abstract_module = py11::module::import("ecole.abstract");

	pyreward::function_class_<reward::IsDone>(m, "IsDone")  //
		.def(py11::init<>());
	pyreward::function_class_<reward::NegLPIterations>(m, "NegLPIterations")  //
		.def(py11::init<>());
}
