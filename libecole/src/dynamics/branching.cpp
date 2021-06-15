#include <algorithm>
#include <stdexcept>

#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/branching.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"

namespace ecole::dynamics {

BranchingDynamics::BranchingDynamics(bool pseudo_candidates_) noexcept : pseudo_candidates(pseudo_candidates_) {}

namespace {

std::optional<xt::xtensor<std::size_t, 1>> action_set(scip::Model const& model, bool pseudo) {
	if (model.get_stage() != SCIP_STAGE_SOLVING) {
		return {};
	}
	auto const branch_cands = pseudo ? model.pseudo_branch_cands() : model.lp_branch_cands();
	auto branch_cols = xt::xtensor<std::size_t, 1>::from_shape({branch_cands.size()});
	auto const var_to_idx = [](auto const var) { return SCIPcolGetLPPos(SCIPvarGetCol(var)); };
	std::transform(branch_cands.begin(), branch_cands.end(), branch_cols.begin(), var_to_idx);

	assert(branch_cols.size() > 0);
	return branch_cols;
}

}  // namespace

auto BranchingDynamics::reset_dynamics(scip::Model& model) -> std::tuple<bool, ActionSet> {
	model.solve_iter();
	if (model.solve_iter_is_done()) {
		return {true, {}};
	}
	return {false, action_set(model, pseudo_candidates)};
}

auto BranchingDynamics::step_dynamics(scip::Model& model, std::size_t const& var_idx) -> std::tuple<bool, ActionSet> {
	auto const lp_cols = model.lp_columns();
	if (var_idx >= lp_cols.size()) {
		throw std::invalid_argument{"Branching index is larger than the number of columns."};
	}
	auto* const var = SCIPcolGetVar(lp_cols[var_idx]);
	scip::call(SCIPbranchVar, model.get_scip_ptr(), var, nullptr, nullptr, nullptr);
	model.solve_iter_branch(SCIP_BRANCHED);

	if (model.solve_iter_is_done()) {
		return {true, {}};
	}
	return {false, action_set(model, pseudo_candidates)};
}

}  // namespace ecole::dynamics
