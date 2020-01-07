#include "ecole/reward.hpp"

namespace ecole {
namespace reward {

std::unique_ptr<base::RewardSpace> Done::clone() const {
	return std::make_unique<Done>(*this);
}

auto Done::get(scip::Model const& model, bool done) -> reward_t {
	(void)model;
	return done ? 1 : 0;
}

}  // namespace reward
}  // namespace ecole
