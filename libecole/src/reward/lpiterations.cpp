#include "ecole/reward/lpiterations.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::reward {

namespace {

auto n_lp_iterations(scip::Model& model) -> std::uint64_t {
	switch (model.get_stage()) {
	// Only stages when the following call is authorized
	case SCIP_STAGE_PRESOLVING:
	case SCIP_STAGE_PRESOLVED:
	case SCIP_STAGE_SOLVING:
	case SCIP_STAGE_SOLVED:
		return static_cast<std::uint64_t>(SCIPgetNLPIterations(model.get_scip_ptr()));
	default:
		return 0;
	}
}

}  // namespace

void LpIterations::before_reset(scip::Model& /*unused*/) {
	last_lp_iter = 0;
}

Reward LpIterations::extract(scip::Model& model, bool /* done */) {
	auto lp_iter_diff = n_lp_iterations(model) - last_lp_iter;
	last_lp_iter += lp_iter_diff;
	return static_cast<double>(lp_iter_diff);
}

}  // namespace ecole::reward
