#include <pybind11/pybind11.h>

#include "ecole/reward.hpp"

#include "base/reward.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(reward, m) {
	m.doc() = "Reward classes for ecole.";

	auto const base_module = py11::module::import("ecole.base");

	py::reward::function_class_<reward::Done>(m, "Done")  //
		.def(py11::init<>());
}
