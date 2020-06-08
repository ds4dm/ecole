#include <cmath>
#include <cstddef>

//#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/struct_branch.h>
//#include <scip/struct_lp.h>
//#include <scip/struct_scip.h>
//#include <scip/struct_var.h>
#include <scip/utils.hpp>

#include "ecole/observation/strongbranchingscores.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace observation {

StrongBranchingScores::StrongBranchingScores() {
	pseudo_cands = true;
}

StrongBranchingScores::StrongBranchingScores(bool pseudo_candidates) {
	pseudo_cands = pseudo_candidates;
}

nonstd::optional<xt::xtensor<double, 1>>
StrongBranchingScores::obtain_observation(environment::State& state) {

	if (state.model.get_stage() == SCIP_STAGE_SOLVING) {

		SCIP* scip = state.model.get_scip_ptr();

		/* store original SCIP parameters */
		bool integralcands = state.model.get_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/integralcands");
		bool scoreall = state.model.get_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/scoreall");
		bool collectscores = state.model.get_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/collectscores");
		bool donotbranch = state.model.get_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/donotbranch");
		bool idempotent = state.model.get_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/idempotent");

		/* set parameters for vanilla full strong branching  */
		if (pseudo_cands) {
			state.model.set_param_explicit<scip::ParamType::Bool>(
				"branching/vanillafullstrong/integralcands", true);
		} else {
			state.model.set_param_explicit<scip::ParamType::Bool>(
				"branching/vanillafullstrong/integralcands", false);
		}
		state.model.set_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/scoreall", true);
		state.model.set_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/collectscores", true);
		state.model.set_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/donotbranch", true);
		state.model.set_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/idempotent", true);

		/* execute vanilla full strong branching */
		SCIP_BRANCHRULE* branchrule = SCIPfindBranchrule(scip, "vanillafullstrong");
		SCIP_Bool allowaddcons = FALSE;
		SCIP_RESULT result;

		scip::call(branchrule->branchexeclp, scip, branchrule, allowaddcons, &result);

		/* get vanilla full strong branching scores */
		SCIP_VAR** cands;
		SCIP_Real* candscores;
		int ncands;

		SCIPgetVanillafullstrongData(scip, &cands, &candscores, &ncands, NULL, NULL);

		assert(ncands >= 0);

		/* restore model parameters */
		state.model.set_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/integralcands", integralcands);
		state.model.set_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/scoreall", scoreall);
		state.model.set_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/collectscores", collectscores);
		state.model.set_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/donotbranch", donotbranch);
		state.model.set_param_explicit<scip::ParamType::Bool>(
			"branching/vanillafullstrong/idempotent", idempotent);

		/* Store strong branching scores in tensor */
		int num_lp_columns = SCIPgetNLPCols(scip);
		auto strong_branching_scores =
			xt::xarray<double>::from_shape({static_cast<std::size_t>(num_lp_columns)});
		strong_branching_scores.fill(std::nan(""));

		SCIP_COL* col;
		int lp_index;
		for (int i = 0; i < ncands; i++) {
			col = SCIPvarGetCol(cands[i]);
			lp_index = SCIPcolGetLPPos(col);
			strong_branching_scores(lp_index) = static_cast<double>(candscores[i]);
		}

		return strong_branching_scores;

	} else {
		return {};
	}
}

}  // namespace observation
}  // namespace ecole
