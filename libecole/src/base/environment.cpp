#include "ecole/base/environment.hpp"

namespace ecole {
namespace base {

void RewardSpace::reset(scip::Model const& model) {
	(void)model;
}
auto RewardSpace::step(scip::Model const& model) -> reward_t {
	(void)model;
	return 0;
}

}  // namespace base
}  // namespace ecole
