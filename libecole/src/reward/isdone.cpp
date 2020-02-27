#include "ecole/reward/isdone.hpp"

namespace ecole {
namespace reward {

auto IsDone::clone() const -> std::unique_ptr<RewardFunction> {
	return std::make_unique<IsDone>(*this);
}

Reward IsDone::get(environment::State const& state, bool done) {
	(void)state;
	return done ? 1 : 0;
}

}  // namespace reward
}  // namespace ecole
