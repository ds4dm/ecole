#include <pybind11/pybind11.h>

#include "ecole/reward.hpp"

namespace py11 = pybind11;

using namespace ecole;

namespace ecole {
namespace py {

template <typename C>
using reward_space_class = py11::class_<C, base::RewardSpace, std::shared_ptr<C>>;

}
}  // namespace ecole

PYBIND11_MODULE(reward, m) {
	m.doc() = "Reward classes for ecole.";

	auto const base_module = py11::module::import("ecole.base");

	py::reward_space_class<reward::Done>(m, "Done")  //
		.def(py11::init<>());
}
