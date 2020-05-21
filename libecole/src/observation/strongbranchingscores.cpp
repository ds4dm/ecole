#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <iostream>
#include <xtensor/xview.hpp>

#include "ecole/observation/strongbranchingscores.hpp"
#include "ecole/scip/type.hpp"


//#include <fmt/format.h>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/utils.hpp>

#include <scip/struct_scip.h>
#include <scip/struct_var.h>
#include <scip/struct_branch.h>
 
#include <scip/struct_lp.h>

namespace ecole {
namespace observation {


using double_tensor = decltype(StrongBranchingScoresObs::candidate_scores);
using int_tensor = decltype(StrongBranchingScoresObs::candidates);

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
	
	/* Store data in struct*/
	double_tensor candidate_scores{{static_cast<unsigned long>(ncands), 1}, 0.};
	int_tensor candidates{{static_cast<unsigned long>(ncands), 1}, 0};

	SCIP_COL * col;
	int lp_index;
	for (int i = 0; i < ncands; i++) {
		col = SCIPvarGetCol(cands[i]);
		lp_index = SCIPcolGetLPPos(col);
		candidates(i, 0) = static_cast<int>(lp_index);
		candidate_scores(i, 0) = static_cast<double>(candscores[i]);

	}

	BranchingResults branching_results;
	branching_results.candidate_scores = candidate_scores;
	branching_results.candidates = candidates;
	branching_results.num_candidates = ncands;
	branching_results.best_candidate = bestcand;

	return branching_results;

}



auto StrongBranchingScores::obtain_observation(environment::State const& state)
	-> nonstd::optional<StrongBranchingScoresObs> {

	BranchingResults branching_results = extract_strong_branching_scores(state.model);
	if (state.model.get_stage() == SCIP_STAGE_SOLVING) {
		return StrongBranchingScoresObs{
			branching_results.candidate_scores, branching_results.candidates, branching_results.num_candidates, branching_results.best_candidate};
	} else {
		return {};
	}
}

}  // namespace observation
}  // namespace ecole
