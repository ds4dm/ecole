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

	if (model.get_stage() == SCIP_STAGE_SOLVING) {

		SCIP* scip = model.get_scip_ptr();

		SCIP_VAR** cands;
		SCIP_Real* cands_lp_values;
		int nb_cands;

		/* get branching candidates */
		SCIPgetLPBranchCands(scip, &cands, &cands_lp_values, NULL, NULL, &nb_cands, NULL);
		assert(nb_cands > 0);

		/* Store pseudocosts in tensor */
		auto const nb_lp_columns = static_cast<std::size_t>(SCIPgetNLPCols(scip));
		auto pseudocosts = xt::xarray<double>::from_shape({nb_lp_columns});
		pseudocosts.fill(std::nan(""));

		SCIP_COL* col;
		SCIP_Real score;
		int lp_index;
		for (int i = 0; i < nb_cands; i++) {
			col = SCIPvarGetCol(cands[i]);
			lp_index = SCIPcolGetLPPos(col);
			score = SCIPgetVarPseudocostScore(scip, cands[i], cands_lp_values[i]);

			pseudocosts(lp_index) = static_cast<double>(score);
		}
		return pseudocosts;

	} else {
		return {};
	}
}

}  // namespace observation
}  // namespace ecole
