#include "ecole/env/environment.hpp"

namespace ecole {
namespace env {

void RewardSpace::reset(scip::Model const& model) { (void)model; }
auto RewardSpace::step(scip::Model const& model) -> reward_t {
	(void)model;
	return 0;
}

} // namespace env
} // namespace ecole
