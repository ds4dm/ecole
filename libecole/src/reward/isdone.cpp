#include "ecole/reward/isdone.hpp"

namespace ecole::reward {

Reward IsDone::extract(scip::Model& /*model*/, bool done) {
	return done ? 1 : 0;
}

}  // namespace ecole::reward
