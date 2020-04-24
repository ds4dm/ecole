#include "ecole/reward/neglpiterations.hpp"

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

void NegLPIterations::reset(environment::State const& initial_state) {
	last_lp_iter = n_lp_iterations(initial_state.model);
}

Reward NegLPIterations::obtain_reward(environment::State const& state, bool) {
	auto lp_iter_diff = n_lp_iterations(state.model) - last_lp_iter;
	last_lp_iter += lp_iter_diff;
	return static_cast<double>(-lp_iter_diff);
}

}  // namespace reward
}  // namespace ecole
