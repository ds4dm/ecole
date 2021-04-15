#include <cmath>
#include <cstddef>

#include <nonstd/span.hpp>
#include <range/v3/view/zip.hpp>
#include <scip/scipdefplugins.h>
#include <scip/struct_branch.h>

#include "ecole/observation/strongbranchingscores.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/utils.hpp"

namespace ecole::observation {

namespace views = ranges::views;

namespace {

/** get vanilla full strong branching scores and variables */
auto scip_get_vanillafullstrong_data(SCIP* const scip) noexcept {
	SCIP_VAR** cands = nullptr;
	SCIP_Real* cands_scores = nullptr;
	int n_cands = 0;
	SCIPgetVanillafullstrongData(scip, &cands, &cands_scores, &n_cands, nullptr, nullptr);
	return std::tuple{
		nonstd::span{cands, static_cast<std::size_t>(n_cands)},
		nonstd::span{cands_scores, static_cast<std::size_t>(n_cands)},
	};
}

}  // namespace

StrongBranchingScores::StrongBranchingScores(bool pseudo_candidates_) : pseudo_candidates(pseudo_candidates_) {}

std::optional<xt::xtensor<double, 1>> StrongBranchingScores::extract(scip::Model& model, bool /* done */) {
	if (model.get_stage() != SCIP_STAGE_SOLVING) {
		return {};
	}

	auto* const scip = model.get_scip_ptr();

	/* store original SCIP parameters */
	auto const integralcands = model.get_param<bool>("branching/vanillafullstrong/integralcands");
	auto const scoreall = model.get_param<bool>("branching/vanillafullstrong/scoreall");
	auto const collectscores = model.get_param<bool>("branching/vanillafullstrong/collectscores");
	auto const donotbranch = model.get_param<bool>("branching/vanillafullstrong/donotbranch");
	auto const idempotent = model.get_param<bool>("branching/vanillafullstrong/idempotent");

	/* set parameters for vanilla full strong branching  */
	model.set_param("branching/vanillafullstrong/integralcands", pseudo_candidates);
	model.set_param("branching/vanillafullstrong/scoreall", true);
	model.set_param("branching/vanillafullstrong/collectscores", true);
	model.set_param("branching/vanillafullstrong/donotbranch", true);
	model.set_param("branching/vanillafullstrong/idempotent", true);

	/* execute vanilla full strong branching */
	auto* branchrule = SCIPfindBranchrule(scip, "vanillafullstrong");
	SCIP_RESULT result;
	scip::call(branchrule->branchexeclp, scip, branchrule, false, &result);
	assert(result == SCIP_DIDNOTRUN);
	auto const [cands, cands_scores] = scip_get_vanillafullstrong_data(scip);

	/* restore model parameters */
	model.set_param("branching/vanillafullstrong/integralcands", integralcands);
	model.set_param("branching/vanillafullstrong/scoreall", scoreall);
	model.set_param("branching/vanillafullstrong/collectscores", collectscores);
	model.set_param("branching/vanillafullstrong/donotbranch", donotbranch);
	model.set_param("branching/vanillafullstrong/idempotent", idempotent);

	/* Store strong branching scores in tensor */
	auto const num_lp_columns = static_cast<std::size_t>(SCIPgetNLPCols(scip));
	auto strong_branching_scores = xt::xtensor<double, 1>({num_lp_columns}, std::nan(""));

	for (auto const [var, score] : views::zip(cands, cands_scores)) {
		auto const lp_index = static_cast<std::size_t>(SCIPcolGetLPPos(SCIPvarGetCol(var)));
		strong_branching_scores[lp_index] = static_cast<double>(score);
	}

	return strong_branching_scores;
}

}  // namespace ecole::observation
