#include "ecole/reward/isdone.hpp"

namespace ecole {
namespace reward {

Reward IsDone::obtain_reward(environment::State const& state, bool done) {
	(void)state;
	return done ? 1 : 0;
}

}  // namespace reward
}  // namespace ecole
