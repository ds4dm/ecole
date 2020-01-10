#include <pybind11/pybind11.h>

#include "ecole/reward.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(reward, m) {
	m.doc() = "Reward classes for ecole.";

	auto base_module = py11::module::import("ecole.base");

	py11::class_<reward::Done, base::RewardSpace>(m, "Done")  //
		.def(py11::init<>());
}
