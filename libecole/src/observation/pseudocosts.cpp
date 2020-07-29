#include <cmath>
#include <cstddef>

#include <nonstd/optional.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "ecole/observation/pseudocosts.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/type.hpp"

namespace ecole {
namespace observation {

nonstd::optional<xt::xtensor<double, 1>> Pseudocosts::obtain_observation(scip::Model& model) {
	if (model.get_stage() != SCIP_STAGE_SOLVING) {
		return {};
	}

	auto* const scip = model.get_scip_ptr();

	/* get branching candidates */
	SCIP_VAR** cands;
	SCIP_Real* cands_lp_values;
	int nb_cands;

	SCIPgetLPBranchCands(scip, &cands, &cands_lp_values, NULL, NULL, &nb_cands, NULL);
	assert(nb_cands > 0);

	/* Store pseudocosts in tensor */
	auto const nb_lp_columns = static_cast<std::size_t>(SCIPgetNLPCols(scip));
	xt::xtensor<double, 1> pseudocosts({nb_lp_columns}, std::nan(""));

	for (std::size_t i = 0; i < static_cast<std::size_t>(nb_cands); i++) {
		auto* const col = SCIPvarGetCol(cands[i]);
		auto const lp_index = static_cast<std::size_t>(SCIPcolGetLPPos(col));
		auto const score = SCIPgetVarPseudocostScore(scip, cands[i], cands_lp_values[i]);
		pseudocosts[lp_index] = static_cast<double>(score);
	}

	return pseudocosts;
}

}  // namespace observation
}  // namespace ecole
