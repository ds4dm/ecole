#include <algorithm>
#include <memory>
#include <utility>

#include <fmt/format.h>
#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/primalsearch.hpp"
#include "ecole/exception.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"

namespace ecole::dynamics {

PrimalSearchDynamics::PrimalSearchDynamics(
	int trials_per_node_,
	int depth_freq_,
	int depth_start_,
	int depth_stop_) noexcept(false) :
	trials_per_node(trials_per_node_), depth_freq(depth_freq_), depth_start(depth_start_), depth_stop(depth_stop_) {
	if (trials_per_node < -1) {
		throw Exception{fmt::format("Illegal value for number of trials per node: {}.", trials_per_node)};
	}
}

namespace {

std::optional<VarIds> action_set(scip::Model const& model) {
	if (model.stage() != SCIP_STAGE_SOLVING) {
		return {};
	}
	auto vars = model.pseudo_branch_cands();  // non-fixed discrete variables
	auto var_ids = xt::xtensor<std::size_t, 1>::from_shape({vars.size()});
	std::transform(  //
		vars.begin(),
		vars.end(),
		var_ids.begin(),
		[](auto const var) { return SCIPvarGetProbindex(var); });

	return var_ids;
}

}  // namespace

auto PrimalSearchDynamics::reset_dynamics(scip::Model& model) -> std::tuple<bool, ActionSet> {
	model.solve_iter_start_primalsearch(trials_per_node, depth_freq, depth_start, depth_stop);
	auto const done = model.solve_iter_is_done();
	if (done) {
		return {done, {}};
	}
	return {done, action_set(model)};
}

auto PrimalSearchDynamics::step_dynamics(scip::Model& model, VarIdVals const& action) -> std::tuple<bool, ActionSet> {
	// transform (var_id, value) pairs into (var, value) pairs
	auto problem_vars = model.variables();
	auto varvals = std::vector<std::pair<SCIP_VAR*, SCIP_Real>>(action.size());
	std::transform(  //
		action.begin(),
		action.end(),
		varvals.begin(),
		[&problem_vars](auto& pair) -> decltype(varvals)::value_type {
			auto& var_id = pair.first;
			auto& val = pair.second;
			if (var_id >= problem_vars.size()) {
				throw Exception{fmt::format("Variable index {} is out of range.", var_id)};
			}
			return {problem_vars[var_id], val};
		});

	model.solve_iter_primalsearch(varvals);

	auto const done = model.solve_iter_is_done();
	if (done) {
		return {done, {}};
	}
	return {done, action_set(model)};
}

}  // namespace ecole::dynamics
