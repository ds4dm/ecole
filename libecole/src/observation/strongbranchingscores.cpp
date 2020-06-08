#include <cmath>
#include <cstddef>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/struct_branch.h>
#include <scip/struct_lp.h>
#include <scip/struct_scip.h>
#include <scip/struct_var.h>
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

auto StrongBranchingScores::obtain_observation(environment::State& state)
	-> nonstd::optional<StrongBranchingScoresObs> {

	if (state.model.get_stage() == SCIP_STAGE_SOLVING) {

		SCIP* scip = state.model.get_scip_ptr();

		/* store original SCIP parameters */
		SCIP_Bool integralcands;
		SCIP_Bool scoreall;
		SCIP_Bool collectscores;
		SCIP_Bool donotbranch;
		SCIP_Bool idempotent;

		scip::call(
			SCIPgetBoolParam,
			scip,
			"branching/vanillafullstrong/integralcands",
			&integralcands);
		scip::call(SCIPgetBoolParam, scip, "branching/vanillafullstrong/scoreall", &scoreall);
		scip::call(
			SCIPgetBoolParam,
			scip,
			"branching/vanillafullstrong/collectscores",
			&collectscores);
		scip::call(
			SCIPgetBoolParam, scip, "branching/vanillafullstrong/donotbranch", &donotbranch);
		scip::call(
			SCIPgetBoolParam, scip, "branching/vanillafullstrong/idempotent", &idempotent);

		/* set parameters for vanilla full strong branching  */
		if (pseudo_cands) {
			scip::call(
				SCIPsetBoolParam, scip, "branching/vanillafullstrong/integralcands", true);
		} else {
			scip::call(
				SCIPsetBoolParam, scip, "branching/vanillafullstrong/integralcands", false);
		}
		scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/scoreall", true);
		scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/collectscores", true);
		scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/donotbranch", true);

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
		scip::call(
			SCIPsetBoolParam, scip, "branching/vanillafullstrong/integralcands", integralcands);
		scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/scoreall", scoreall);
		scip::call(
			SCIPsetBoolParam, scip, "branching/vanillafullstrong/collectscores", collectscores);
		scip::call(
			SCIPsetBoolParam, scip, "branching/vanillafullstrong/donotbranch", donotbranch);
		scip::call(
			SCIPsetBoolParam, scip, "branching/vanillafullstrong/idempotent", idempotent);

		/* Store strong branching scores in tensor */
		int num_lp_columns = SCIPgetNLPCols(scip);
		auto strong_branching_scores =
			xt::xarray<double>::from_shape({static_cast<unsigned long>(num_lp_columns)});
		strong_branching_scores.fill(std::nan(""));

		SCIP_COL* col;
		int lp_index;
		for (int i = 0; i < ncands; i++) {
			col = SCIPvarGetCol(cands[i]);
			lp_index = SCIPcolGetLPPos(col);
			strong_branching_scores(lp_index) = static_cast<double>(candscores[i]);
		}

		return StrongBranchingScoresObs{strong_branching_scores};

	} else {
		return {};
	}
}

}  // namespace observation
}  // namespace ecole
