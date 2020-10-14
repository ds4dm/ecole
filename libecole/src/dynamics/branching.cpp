#include <algorithm>
#include <memory>
#include <utility>

#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/branching.hpp"
#include "ecole/exception.hpp"
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
	std::transform(  //
		branch_cands.begin(),
		branch_cands.end(),
		branch_cols.begin(),
		[](auto const var) { return SCIPcolGetLPPos(SCIPvarGetCol(var)); });

	assert(branch_cols.size() > 0);
	return branch_cols;
}

}  // namespace

auto BranchingDynamics::reset_dynamics(scip::Model& model) -> std::tuple<bool, ActionSet> {
	model.solve_iter();
	auto const done = model.solve_iter_is_done();
	if (done) {
		return {done, {}};
	}
	return {done, action_set(model, pseudo_candidates)};
}

auto BranchingDynamics::step_dynamics(scip::Model& model, std::size_t const& action) -> std::tuple<bool, ActionSet> {
	auto const lp_cols = model.lp_columns();
	if (action >= lp_cols.size()) {
		throw Exception{"Branching index is larger than the number of columns."};
	}
	model.solve_iter_branch(SCIPcolGetVar(lp_cols[action]));

	auto const done = model.solve_iter_is_done();
	if (done) {
		return {done, {}};
	}
	return {done, action_set(model, pseudo_candidates)};
}

}  // namespace ecole::dynamics
