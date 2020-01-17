#include "ecole/base.hpp"

namespace ecole {
namespace base {

void RewardSpace::reset(scip::Model const& model) {
	(void)model;
}

void TerminationSpace::reset(scip::Model const& model) {
	(void)model;
}

}  // namespace base
}  // namespace ecole
