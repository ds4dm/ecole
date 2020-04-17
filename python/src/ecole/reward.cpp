#include <pybind11/pybind11.h>

#include "ecole/reward/isdone.hpp"
#include "ecole/reward/neglpiterations.hpp"

namespace py = pybind11;
using namespace ecole;

template <typename RewardFunction>
auto reward_function_class(py::module& m, char const* name) {
	return py::class_<RewardFunction>(m, name)  //
		.def(py::init<>())
		.def("reset", &RewardFunction::reset, py::arg("state"))
		.def("get", &RewardFunction::get, py::arg("state"), py::arg("done") = false);
}

PYBIND11_MODULE(reward, m) {
	m.doc() = "Reward classes for ecole.";

	reward_function_class<reward::IsDone>(m, "IsDone");

	reward_function_class<reward::NegLPIterations>(m, "NegLPIterations");
}
