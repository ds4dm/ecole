#include "ecole/reward/isdone.hpp"

namespace ecole {
namespace reward {

std::unique_ptr<RewardFunction> IsDone::clone() const {
	return std::make_unique<IsDone>(*this);
}

auto IsDone::get(scip::Model const& model, bool done) -> reward_t {
	(void)model;
	return done ? 1 : 0;
}

}  // namespace reward
}  // namespace ecole
