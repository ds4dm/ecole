#include "ecole/reward/isdone.hpp"

namespace ecole {
namespace reward {

std::unique_ptr<RewardFunction> IsDone::clone() const {
	return std::make_unique<IsDone>(*this);
}

Reward IsDone::get(scip::Model const& model, bool done) {
	(void)model;
	return done ? 1 : 0;
}

}  // namespace reward
}  // namespace ecole
