#include "ecole/reward/lpiterations.hpp"

#include "ecole/scip/model.hpp"

namespace ecole {
namespace reward {

static auto n_lp_iterations(scip::Model const& model) {
	switch (model.get_stage()) {
	// Only stages when the following call is authorized
	case SCIP_STAGE_PRESOLVING:
	case SCIP_STAGE_PRESOLVED:
	case SCIP_STAGE_SOLVING:
	case SCIP_STAGE_SOLVED:
		return SCIPgetNLPIterations(model.get_scip_ptr());
	default:
		return decltype(SCIPgetNLPIterations(nullptr)){0};
	}
}

void LpIterations::reset(scip::Model const& model) {
	last_lp_iter = n_lp_iterations(model);
}

Reward LpIterations::obtain_reward(scip::Model const& model, bool /* done */) {
	auto lp_iter_diff = n_lp_iterations(model) - last_lp_iter;
	last_lp_iter += lp_iter_diff;
	return static_cast<double>(-lp_iter_diff);
}

}  // namespace reward
}  // namespace ecole
