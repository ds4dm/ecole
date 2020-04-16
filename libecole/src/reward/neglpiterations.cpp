#include "ecole/reward/neglpiterations.hpp"

namespace ecole {
namespace reward {

auto NegLPIterations::clone() const -> std::unique_ptr<RewardFunction> {
	return std::make_unique<NegLPIterations>(*this);
}

void NegLPIterations::reset(environment::State const& initial_state) {
	last_lp_iter = SCIPgetNLPIterations(initial_state.model.get_scip_ptr());
}

Reward NegLPIterations::get(environment::State const& state, bool done) {
	(void)done;
	auto lp_iter_diff = SCIPgetNLPIterations(state.model.get_scip_ptr()) - last_lp_iter;
	last_lp_iter += lp_iter_diff;
	return static_cast<double>(-lp_iter_diff);
}

}  // namespace reward
}  // namespace ecole
