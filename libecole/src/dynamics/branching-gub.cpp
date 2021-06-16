/***********************************************************
 *  Implementation of SCIPbranchGUB (aim to merge in SCIP) *
 ***********************************************************/

#include <cassert>

#include <fmt/format.h>

#include <scip/cons_linear.h>
#include <scip/debug.h>
#include <scip/scip.h>
#include <scip/scip_branch.h>
#include <scip/scip_numerics.h>
#include <scip/struct_scip.h>
#include <scip/struct_var.h>

namespace {

SCIP_RETCODE SCIPbranchGUB_AddChild(
	SCIP* scip,
	SCIP_Real priority,
	SCIP_Real estimate,
	SCIP_VAR** vars,
	SCIP_Real* ones,
	int nvars,
	SCIP_Real lhs,
	SCIP_Real rhs,
	SCIP_NODE** node_out) {
	SCIP_NODE* node = nullptr;
	SCIP_CALL(SCIPcreateChild(scip, &node, priority, estimate));
	auto name = fmt::format("branching-{}", SCIPnodeGetNumber(node));
	SCIP_CONS* cons = nullptr;
	SCIP_CALL(SCIPcreateConsLinear(
		scip,
		&cons,
		name.c_str(),
		nvars,
		vars,
		ones,
		lhs,
		rhs,
		/*initial=*/TRUE,
		/*separate=*/TRUE,
		/*enforce=*/TRUE,
		/*check=*/FALSE,
		/*propagate=*/TRUE,
		/*local=*/TRUE,
		/*modifiable=*/FALSE,
		/*dynamic=*/FALSE,
		/*removable=*/FALSE,
		/*stickingatnode=*/TRUE));
	SCIP_RETCODE retcode = SCIP_OKAY;
	SCIP_CALL_TERMINATE(retcode, SCIPaddConsNode(scip, node, cons, nullptr), TERM);
	if (node_out != nullptr) {
		*node_out = node;
	}

TERM:
	SCIP_CALL(SCIPreleaseCons(scip, &cons));
	return retcode;
}

SCIP_RETCODE
SCIPbranchGUB(SCIP* scip, SCIP_VAR** vars, int nvars, SCIP_NODE** downchild, SCIP_NODE** upchild) {
	/* Check input parameters */
	assert(scip != nullptr);
	if (SCIPgetStage(scip) != SCIP_STAGE_SOLVING) {
		SCIPerrorMessage("cannot branch when not solving\n");
		return SCIP_INVALIDCALL;
	}
	if (nvars <= 0) {
		SCIPerrorMessage("cannot branch on empty variable set\n");
		return SCIP_INVALIDDATA;
	}

	/* Check individual variables and compute the sum of their LP relaxation value */
	SCIP_Real pseudo_sol_sum = 0.;
	SCIP_VAR const* const* const vars_end = vars + nvars;
	for (SCIP_VAR** var_iter = vars; var_iter < vars_end; ++var_iter) {
		/* Validate that variables are integer, not fixed */
		assert(SCIPvarIsActive(*var_iter));
		assert(SCIPvarGetProbindex(*var_iter) >= 0);
		if (SCIPvarGetType(*var_iter) == SCIP_VARTYPE_CONTINUOUS) {
			SCIPerrorMessage("cannot branch on constraint containing continuous variable <%s>\n", SCIPvarGetName(*var_iter));
			return SCIP_INVALIDDATA;
		}
		if (SCIPisEQ(scip, SCIPvarGetLbLocal(*var_iter), SCIPvarGetUbLocal(*var_iter))) {
			SCIPerrorMessage(
				"cannot branch on constraint containing variable <%s> with fixed domain [%.15g,%.15g]\n",
				SCIPvarGetName(*var_iter),
				SCIPvarGetLbLocal(*var_iter),
				SCIPvarGetUbLocal(*var_iter));
			return SCIP_INVALIDDATA;
		}

		SCIP_Real val = SCIPvarGetSol(*var_iter, SCIPhasCurrentNodeLP(scip));

		/* avoid branching on infinite values in pseudo solution */
		if (SCIPisInfinity(scip, -val) || SCIPisInfinity(scip, val)) {
			SCIPerrorMessage("cannot branch on variables containing infinite values");
			return SCIP_INVALIDDATA;
		}
		pseudo_sol_sum += val;
	}

	SCIP_Real const downbound = SCIPfeasFloor(scip, pseudo_sol_sum);
	SCIP_Real const upbound = SCIPfeasCeil(scip, pseudo_sol_sum);
	SCIP_Real const estimate = SCIPnodeGetLowerbound(SCIPgetCurrentNode(scip));
	if (SCIPisEQ(scip, downbound, upbound)) {
		SCIPerrorMessage("cannot branch on a variables whose sum of LP solution value is integer");
		return SCIP_INVALIDDATA;
	}

	SCIP_Real const inf = SCIPinfinity(scip);
	SCIP_Real* ones = nullptr;
	SCIP_Retcode retcode = SCIP_OKAY;
	SCIP_CALL(SCIPallocBufferArray(scip, &ones, nvars));
	for (int i = 0; i < nvars; ++i) {
		ones[i] = 1.;
	}

	SCIP_CALL_TERMINATE(
		retcode, SCIPbranchGUB_AddChild(scip, 1, estimate, vars, ones, nvars, -inf, downbound, downchild), TERM);
	SCIP_CALL_TERMINATE(
		retcode, SCIPbranchGUB_AddChild(scip, 1, estimate, vars, ones, nvars, upbound, inf, upchild), TERM);

TERM:
	SCIPfreeBufferArray(scip, &ones);
	return retcode;
}

}  // namespace

/********************************************
 *  Implementation of BranchingGUBDynamics  *
 ********************************************/

#include <algorithm>
#include <stdexcept>
#include <vector>

#include <xtensor/xtensor.hpp>

#include "ecole/dynamics/branching-gub.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"

namespace ecole::dynamics {

namespace {

std::optional<xt::xtensor<std::size_t, 1>> action_set(scip::Model const& model) {
	if (model.get_stage() != SCIP_STAGE_SOLVING) {
		return {};
	}
	auto const branch_cands = model.lp_branch_cands();
	auto branch_cols = xt::xtensor<std::size_t, 1>::from_shape({branch_cands.size()});
	auto const var_to_idx = [](auto const var) { return SCIPcolGetLPPos(SCIPvarGetCol(var)); };
	std::transform(branch_cands.begin(), branch_cands.end(), branch_cols.begin(), var_to_idx);

	assert(branch_cols.size() > 0);
	return branch_cols;
}

}  // namespace

auto BranchingGUBDynamics::reset_dynamics(scip::Model& model) -> std::tuple<bool, ActionSet> {
	model.solve_iter();
	if (model.solve_iter_is_done()) {
		return {true, {}};
	}
	return {false, action_set(model)};
}

auto BranchingGUBDynamics::step_dynamics(scip::Model& model, Action const& var_indices) -> std::tuple<bool, ActionSet> {
	auto const lp_cols = model.lp_columns();

	// Check that input indices are within range
	auto const is_out_of_bounds = [size = lp_cols.size()](auto idx) { return idx >= size; };
	if (std::any_of(var_indices.begin(), var_indices.end(), is_out_of_bounds)) {
		throw std::invalid_argument{"Branching index is larger than the number of columns."};
	}

	{  // Get variables associated with indices and branch
		auto vars = std::vector<SCIP_VAR*>(var_indices.size());
		auto const idx_to_var = [lp_cols](auto idx) { return SCIPcolGetVar(lp_cols[idx]); };
		std::transform(var_indices.begin(), var_indices.end(), vars.begin(), idx_to_var);

		scip::call(SCIPbranchGUB, model.get_scip_ptr(), vars.data(), static_cast<int>(vars.size()), nullptr, nullptr);
		model.solve_iter_branch(SCIP_BRANCHED);
	}

	if (model.solve_iter_is_done()) {
		return {true, {}};
	}
	return {false, action_set(model)};
}

}  // namespace ecole::dynamics
