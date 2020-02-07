#include "ecole/reward/isdone.hpp"

namespace ecole {
namespace reward {

auto IsDone::clone() const -> std::unique_ptr<RewardFunction<State>> {
	return std::make_unique<IsDone>(*this);
}

Reward IsDone::get(State const& state, bool done) {
	(void)state;
	return done ? 1 : 0;
}

}  // namespace reward
}  // namespace ecole
