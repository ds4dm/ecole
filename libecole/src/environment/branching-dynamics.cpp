#include <algorithm>
#include <memory>
#include <utility>

#include <nonstd/span.hpp>
#include <xtensor/xtensor.hpp>

#include "ecole/environment/branching-dynamics.hpp"
#include "ecole/environment/exception.hpp"
#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"

namespace ecole {
namespace environment {

BranchingDynamics::BranchingDynamics(bool pseudo_candidates_) noexcept :
	pseudo_candidates(pseudo_candidates_) {}

static nonstd::span<SCIP_VAR*> lp_branch_cands(SCIP* scip) {
	SCIP_VAR** cands = nullptr;
	int n_cands = 0;
	scip::call(
		SCIPgetLPBranchCands, scip, &cands, nullptr, nullptr, &n_cands, nullptr, nullptr);
	assert(n_cands >= 0);
	return {cands, static_cast<std::size_t>(n_cands)};
}

static nonstd::span<SCIP_VAR*> pseudo_branch_cands(SCIP* scip) {
	SCIP_VAR** cands = nullptr;
	int n_cands = 0;
	scip::call(SCIPgetPseudoBranchCands, scip, &cands, &n_cands, nullptr);
	assert(n_cands >= 0);
	return {cands, static_cast<std::size_t>(n_cands)};
}

static nonstd::optional<xt::xtensor<std::size_t, 1>>
action_set(scip::Model const& model, bool pseudo) {
	if (model.get_stage() != SCIP_STAGE_SOLVING) {
		return {};
	}
	auto const scip = model.get_scip_ptr();
	auto const branch_cands = pseudo ? pseudo_branch_cands(scip) : lp_branch_cands(scip);
	auto branch_cols = xt::xtensor<std::size_t, 1>::from_shape({branch_cands.size()});
	std::transform(  //
		branch_cands.begin(),
		branch_cands.end(),
		branch_cols.begin(),
		[](auto const var) { return SCIPcolGetLPPos(SCIPvarGetCol(var)); });

	assert(branch_cols.size() > 0);
	return branch_cols;
}

auto BranchingDynamics::reset_dynamics(State& init_state) -> std::tuple<bool, ActionSet> {
	init_state.model.solve_iter();
	return {
		init_state.model.solve_iter_is_done(),
		action_set(init_state.model, pseudo_candidates)};
}

auto BranchingDynamics::step_dynamics(State& state, std::size_t const& action)
	-> std::tuple<bool, ActionSet> {
	auto const lp_cols = state.model.lp_columns();
	if (action >= lp_cols.size) {
		throw Exception("");
	}
	state.model.solve_iter_branch(lp_cols[action].var());

	return {state.model.solve_iter_is_done(), action_set(state.model, pseudo_candidates)};
}

}  // namespace environment
}  // namespace ecole
