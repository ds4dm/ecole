#include "ecole/termination/whensolved.hpp"

namespace ecole {
namespace termination {

std::unique_ptr<TerminationFunction> WhenSolved::clone() const {
	return std::make_unique<WhenSolved>(*this);
}

bool WhenSolved::is_done(scip::Model const& model) {
	return model.is_solved();
}

}  // namespace termination
}  // namespace ecole
