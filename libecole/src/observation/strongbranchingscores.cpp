#include <cstddef>
#include <cmath>

#include "ecole/observation/strongbranchingscores.hpp"
#include "ecole/scip/type.hpp"

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/utils.hpp>
#include <scip/struct_scip.h>
#include <scip/struct_var.h>
#include <scip/struct_branch.h>
#include <scip/struct_lp.h>

namespace ecole {
namespace observation {

static auto extract_strong_branching_scores(scip::Model const& model) 
{
	
	/* store original SCIP parameters */
	bool integralcands = model.get_param_explicit<scip::ParamType::Bool>("branching/vanillafullstrong/integralcands");
	bool scoreall = model.get_param_explicit<scip::ParamType::Bool>("branching/vanillafullstrong/scoreall");
	bool collectscores = model.get_param_explicit<scip::ParamType::Bool>("branching/vanillafullstrong/collectscores");
	bool donotbranch = model.get_param_explicit<scip::ParamType::Bool>("branching/vanillafullstrong/donotbranch");
	bool idempotent = model.get_param_explicit<scip::ParamType::Bool>("branching/vanillafullstrong/idempotent");
	
	/* set parameters for vanilla full strong branching  */
	SCIP * scip = model.get_scip_ptr();
	
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/integralcands", true);
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/scoreall", true);
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/collectscores", true);
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/donotbranch", true);
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/idempotent", true);
	
	/* execute vanilla full strong branching */
	SCIP_BRANCHRULE * branchrule = SCIPfindBranchrule(scip, "vanillafullstrong");
	SCIP_Bool allowaddcons = TRUE;
	SCIP_RESULT result = SCIP_FEASIBLE;
	
 	branchrule->branchexeclp(scip, branchrule, allowaddcons, & result);

 	/* get vanilla full strong branching scores */
    SCIP_VAR** cands;
    SCIP_Real* candscores;
    int ncands;
    int npriocands;
    int bestcand;

    SCIPgetVanillafullstrongData(
    	scip,
		& cands,
		& candscores,
		& ncands,
		& npriocands,
		& bestcand 
	);

	assert(ncands >= 0);
	assert(npriocands >= 0);
	assert(ncands == npriocands);
	assert(bestcand > -1);

	/* restore model parameters */	
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/integralcands", integralcands);
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/scoreall", scoreall);
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/collectscores", collectscores);
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/donotbranch", donotbranch);
	scip::call(SCIPsetBoolParam, scip, "branching/vanillafullstrong/idempotent", idempotent);
	
	/* Store strong branching scores in tensor */
	int num_lp_columns = SCIPgetNLPCols(scip);
	auto strong_branching_scores = xt::xarray<double>::from_shape({static_cast<unsigned long>(num_lp_columns)});
	strong_branching_scores.fill(std::nan(""));

	SCIP_COL * col;
	int lp_index;
	for (int i = 0; i < ncands; i++) {
		col = SCIPvarGetCol(cands[i]);
		lp_index = SCIPcolGetLPPos(col);
		strong_branching_scores(lp_index) = static_cast<double>(candscores[i]);
	}

	return strong_branching_scores;

}



auto StrongBranchingScores::obtain_observation(environment::State const& state)
	-> nonstd::optional<StrongBranchingScoresObs> {

	if (state.model.get_stage() == SCIP_STAGE_SOLVING) {
		return StrongBranchingScoresObs{
			extract_strong_branching_scores(state.model)};
	} else {
		return {};
	}
}

}  // namespace observation
}  // namespace ecole
