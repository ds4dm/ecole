#include "ecole/termination/base.hpp"

namespace ecole {
namespace termination {

void TerminationFunction::reset(scip::Model const& model) {
	(void)model;
}

}  // namespace termination
}  // namespace ecole
