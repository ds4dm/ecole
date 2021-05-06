#include <fmt/format.h>

#include <scip/cons_linear.h>
#include <scip/debug.h>
#include <scip/scip.h>
#include <scip/scip_branch.h>
#include <scip/scip_numerics.h>
#include <scip/struct_scip.h>
#include <scip/struct_var.h>

#include "scip/gub-branch.hpp"

static SCIP_RETCODE SCIPbranchGUB_validate(SCIP* scip, SCIP_VAR** vars, int nvars) {
	assert(scip != nullptr);
	if (SCIPgetStage(scip) != SCIP_STAGE_SOLVING) {
		SCIPerrorMessage("cannot branch when not solving\n");
		return SCIP_INVALIDCALL;
	}

	if (nvars <= 0) {
		SCIPerrorMessage("cannot branch on empty variable set\n");
		return SCIP_INVALIDDATA;
	}

	SCIP_VAR const* const* const vars_end = vars + nvars;
	for (SCIP_VAR** var_iter = vars; var_iter < vars_end; ++var_iter) {
		// assert((*var_iter)->scip == scip);  FIXME only in SCIP debug builds
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
	}

	return SCIP_OKAY;
}

static SCIP_RETCODE SCIPbranchGUB_add_child(
	SCIP* scip,
	SCIP_VAR** vars,
	SCIP_Real* ones,
	int nvars,
	SCIP_Real lhs,
	SCIP_Real rhs,
	SCIP_NODE** node_out) {
	// TODO decide on priorities
	SCIP_NODE* node = nullptr;
	SCIP_CALL(SCIPcreateChild(scip, &node, 1., 0.));
	auto name = fmt::format("branching-{}", SCIPnodeGetNumber(node));
	SCIP_CONS* cons = nullptr;
	SCIP_CALL(SCIPcreateConsBasicLinear(scip, &cons, name.c_str(), nvars, vars, ones, lhs, rhs));
	SCIP_CALL(SCIPaddConsNode(scip, node, cons, nullptr));
	if (node_out != nullptr) {
		*node_out = node;
	}
	return SCIP_OKAY;
}

SCIP_RETCODE
SCIPbranchGUB(SCIP* scip, SCIP_VAR** vars, int nvars, SCIP_NODE** downchild, SCIP_NODE** eqchild, SCIP_NODE** upchild) {
	SCIPbranchGUB_validate(scip, vars, nvars);

	SCIP_Real pseudo_sol_sum = 0.;
	SCIP_VAR const* const* const vars_end = vars + nvars;
	for (SCIP_VAR** var_iter = vars; var_iter < vars_end; ++var_iter) {
		SCIP_Real val = SCIPvarGetSol(*var_iter, SCIPhasCurrentNodeLP(scip));

		/* avoid branching on infinite values in pseudo solution */
		if (SCIPisInfinity(scip, -val) || SCIPisInfinity(scip, val)) {
			// FIXME not sure what to do here
			return SCIP_INVALIDDATA;
		}
		pseudo_sol_sum += val;
	}

	SCIP_Real inf = SCIPinfinity(scip);
	SCIP_Real* ones = nullptr;
	SCIP_Retcode retcode = SCIP_OKAY;
	SCIP_CALL(SCIPallocBufferArray(scip, &ones, nvars));
	for (int i = 0; i < nvars; ++i) {
		ones[i] = 1.;
	}

	SCIP_Real const downbound = SCIPfeasFloor(scip, pseudo_sol_sum);
	SCIP_Real const upbound = SCIPfeasCeil(scip, pseudo_sol_sum);
	if (SCIPisEQ(scip, downbound, upbound)) {
		SCIP_CALL_TERMINATE(retcode, SCIPbranchGUB_add_child(scip, vars, ones, nvars, upbound, upbound, eqchild), TERM);
		SCIP_CALL_TERMINATE(retcode, SCIPbranchGUB_add_child(scip, vars, ones, nvars, -inf, upbound - 1, downchild), TERM);
		SCIP_CALL_TERMINATE(retcode, SCIPbranchGUB_add_child(scip, vars, ones, nvars, upbound + 1, inf, upchild), TERM);
	} else {
		SCIP_CALL_TERMINATE(retcode, SCIPbranchGUB_add_child(scip, vars, ones, nvars, -inf, downbound, downchild), TERM);
		SCIP_CALL_TERMINATE(retcode, SCIPbranchGUB_add_child(scip, vars, ones, nvars, upbound, inf, upchild), TERM);
	}

TERM:
	SCIPfreeBufferArray(scip, &ones);
	return retcode;
}
