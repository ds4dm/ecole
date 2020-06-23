#include <cmath>
#include <cstddef>

#include <scip/scipdefplugins.h>
#include <scip/struct_branch.h>
#include <scip/utils.hpp>

#include "ecole/observation/strongbranchingscores.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace observation {

StrongBranchingScores::StrongBranchingScores(bool pseudo_candidates_) :
	pseudo_candidates(pseudo_candidates_) {}

nonstd::optional<xt::xtensor<double, 1>>
StrongBranchingScores::obtain_observation(scip::Model& model) {

	if (model.get_stage() == SCIP_STAGE_SOLVING) {

		SCIP* scip = model.get_scip_ptr();

		/* store original SCIP parameters */
		auto const integralcands = model.get_param<bool>("branching/vanillafullstrong/integralcands");
		auto const scoreall = model.get_param<bool>("branching/vanillafullstrong/scoreall");
		auto const collectscores = model.get_param<bool>("branching/vanillafullstrong/collectscores");
		auto const donotbranch = model.get_param<bool>("branching/vanillafullstrong/donotbranch");
		auto const idempotent = model.get_param<bool>("branching/vanillafullstrong/idempotent");

		/* set parameters for vanilla full strong branching  */
		if (pseudo_candidates) {
			model.set_param("branching/vanillafullstrong/integralcands", true);
		} else {
			model.set_param("branching/vanillafullstrong/integralcands", false);
		}
		model.set_param("branching/vanillafullstrong/scoreall", true);
		model.set_param("branching/vanillafullstrong/collectscores", true);
		model.set_param("branching/vanillafullstrong/donotbranch", true);
		model.set_param("branching/vanillafullstrong/idempotent", true);

		/* execute vanilla full strong branching */
		SCIP_BRANCHRULE* branchrule = SCIPfindBranchrule(scip, "vanillafullstrong");
		SCIP_RESULT result;
		scip::call(branchrule->branchexeclp, scip, branchrule, false, &result);
		assert(result == SCIP_DIDNOTRUN);

		/* get vanilla full strong branching scores */
		SCIP_VAR** cands;
		SCIP_Real* candscores;
		int ncands;

		SCIPgetVanillafullstrongData(scip, &cands, &candscores, &ncands, NULL, NULL);

		assert(ncands >= 0);

		/* restore model parameters */
		model.set_param("branching/vanillafullstrong/integralcands", integralcands);
		model.set_param("branching/vanillafullstrong/scoreall", scoreall);
		model.set_param("branching/vanillafullstrong/collectscores", collectscores);
		model.set_param("branching/vanillafullstrong/donotbranch", donotbranch);
		model.set_param("branching/vanillafullstrong/idempotent", idempotent);

		/* Store strong branching scores in tensor */
		auto const num_lp_columns = static_cast<std::size_t>(SCIPgetNLPCols(scip));
		xt::xtensor<double, 1> strong_branching_scores({num_lp_columns}, std::nan(""));

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
