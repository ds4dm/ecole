#include "ecole/base.hpp"

namespace ecole {
namespace base {

void RewardFunction::reset(scip::Model const& model) {
	(void)model;
}

void TerminationFunction::reset(scip::Model const& model) {
	(void)model;
}

}  // namespace base
}  // namespace ecole
