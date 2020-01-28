#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <xtensor-python/pytensor.hpp>

#include "ecole/base.hpp"
#include "ecole/branching.hpp"
#include "ecole/observation/basicobs.hpp"
#include "ecole/reward/isdone.hpp"
#include "ecole/termination/whensolved.hpp"

#include "wrapper/environment.hpp"

namespace py11 = pybind11;

using namespace ecole;

PYBIND11_MODULE(branching, m) {
	m.doc() = "Learning to branch task.";
	// Import of base required for resolving inheritance to base types
	py11::module base_mod = py11::module::import("ecole.base");

	using ActionFunction = pyenvironment::ActionFunctionBase<branching::ActionFunction>;
	using Fractional =
		pyenvironment::ActionFunction<branching::Fractional, branching::ActionFunction>;
	using Env = pyenvironment::Env<branching::Environment>;

	py11::class_<ActionFunction, std::shared_ptr<ActionFunction>>(m, "ActionFunction");
	py11::class_<Fractional, ActionFunction, std::shared_ptr<Fractional>>(
		m, "Fractional")  //
		.def(py11::init<>());

	py11::class_<Env, pyenvironment::EnvBase>(m, "Environment")  //
		.def_static(
			"make_dummy",
			[] {
				return std::make_unique<Env>(
					std::make_unique<Fractional>(),
					std::make_unique<pyobservation::ObsFunction<observation::BasicObsFunction>>(),
					std::make_unique<reward::IsDone>(),
					std::make_unique<termination::WhenSolved>());
			})
		.def(py11::init<
				 Env::ptr<ActionFunction> const&,
				 Env::ptr<pyobservation::ObsFunctionBase> const&,
				 Env::ptr<reward::RewardFunction> const&,
				 Env::ptr<termination::TerminationFunction> const&>())  //
		.def(
			"step",
			[](pyenvironment::EnvBase& env, branching::Fractional::action_t const& action) {
				return env.step(pyenvironment::Action<std::size_t>(action));
			});
	;
}
