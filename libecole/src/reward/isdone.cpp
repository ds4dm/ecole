#include "ecole/reward/isdone.hpp"

namespace ecole {
namespace reward {

Reward IsDone::obtain_reward(scip::Model const& /*model*/, bool done) {
	return done ? 1 : 0;
}

}  // namespace reward
}  // namespace ecole
