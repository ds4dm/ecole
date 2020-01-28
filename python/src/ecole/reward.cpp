#include <pybind11/pybind11.h>

#include "ecole/reward/isdone.hpp"

#include "wrapper/reward.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(reward, m) {
	m.doc() = "Reward classes for ecole.";

	auto const base_module = py11::module::import("ecole.base");

	pyreward::function_class_<reward::IsDone>(m, "IsDone")  //
		.def(py11::init<>());
}
