#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace reward {

void RewardFunction::reset(scip::Model const& model) {
	(void)model;
}

}  // namespace reward
}  // namespace ecole
