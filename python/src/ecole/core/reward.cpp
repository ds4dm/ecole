#include <pybind11/pybind11.h>

#include "ecole/reward/constant.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/reward/neglpiterations.hpp"

#include "core.hpp"

namespace ecole {
namespace reward {

namespace py = pybind11;

template <typename RewardFunction>
auto reward_function_class(py::module& m, char const* name) {
	return py::class_<RewardFunction>(m, name)  //
		.def("reset", &RewardFunction::reset, py::arg("state"))
		.def(
			"obtain_reward",
			&RewardFunction::obtain_reward,
			py::arg("state"),
			py::arg("done") = false);
}

void bind_submodule(py::module m) {
	m.doc() = "Reward classes for Ecole.";

	reward_function_class<Constant>(m, "Constant")  //
		.def(py::init<Reward>(), py::arg("constant") = 0.);

	reward_function_class<IsDone>(m, "IsDone")  //
		.def(py::init<>());

	reward_function_class<NegLPIterations>(m, "NegLPIterations")  //
		.def(py::init<>());
}

}  // namespace reward
}  // namespace ecole
