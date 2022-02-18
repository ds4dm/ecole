#include <algorithm>
#include <stdexcept>

#include <fmt/format.h>
#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/branching.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"

namespace ecole::dynamics {

BranchingDynamics::BranchingDynamics(bool pseudo_candidates_) noexcept : pseudo_candidates(pseudo_candidates_) {}

namespace {

auto action_set(scip::Model const& model, bool pseudo) -> std::optional<xt::xtensor<std::size_t, 1>> {
	if (model.stage() != SCIP_STAGE_SOLVING) {
		return {};
	}
	auto const branch_cands = pseudo ? model.pseudo_branch_cands() : model.lp_branch_cands();
	auto branch_cols = xt::xtensor<std::size_t, 1>::from_shape({branch_cands.size()});
	auto const var_to_idx = [](auto const var) { return SCIPvarGetProbindex(var); };
	std::transform(branch_cands.begin(), branch_cands.end(), branch_cols.begin(), var_to_idx);

	assert(branch_cols.size() > 0);
	return branch_cols;
}

/** Iterative solving until next LP branchrule call and return the action_set. */
template <typename FCall>
auto keep_solving_until_next_LP_callback(scip::Model& model, FCall& fcall, bool pseudo_candidates)
	-> std::tuple<bool, BranchingDynamics::ActionSet> {
	using Call = scip::callback::BranchruleCall;
	// While solving is not finished.
	while (fcall.has_value()) {
		// LP branchrule found, we give control back to the agent.
		// Assuming Branchrules are the only reverse callbacks.
		if (std::get<Call>(fcall.value()).where == Call::Where::LP) {
			return {false, action_set(model, pseudo_candidates)};
		}
		// Otherwise keep looping, ignoring the callback.
		fcall = model.solve_iter_continue(SCIP_DIDNOTRUN);
	}
	// Solving is finished.
	return {true, {}};
}

}  // namespace

auto BranchingDynamics::reset_dynamics(scip::Model& model) const -> std::tuple<bool, ActionSet> {
	auto fcall = model.solve_iter(scip::callback::BranchruleConstructor{});
	return keep_solving_until_next_LP_callback(model, fcall, pseudo_candidates);
}

auto BranchingDynamics::step_dynamics(scip::Model& model, Defaultable<std::size_t> maybe_var_idx) const
	-> std::tuple<bool, ActionSet> {
	// Default fallback to SCIP default branching
	auto scip_result = SCIP_DIDNOTRUN;

	if (std::holds_alternative<std::size_t>(maybe_var_idx)) {
		auto const var_idx = std::get<std::size_t>(maybe_var_idx);
		auto const vars = model.variables();
		// Error handling
		if (var_idx >= vars.size()) {
			throw std::invalid_argument{
				fmt::format("Branching candidate index {} larger than the number of variables ({}).", var_idx, vars.size())};
		}
		// Branching
		scip::call(SCIPbranchVar, model.get_scip_ptr(), vars[var_idx], nullptr, nullptr, nullptr);
		scip_result = SCIP_BRANCHED;
	}

	// Looping until the next LP branchrule rule callback, if it exists.
	auto fcall = model.solve_iter_continue(scip_result);
	return keep_solving_until_next_LP_callback(model, fcall, pseudo_candidates);
}

}  // namespace ecole::dynamics
