#include "ecole/termination.hpp"

namespace ecole {
namespace termination {

std::unique_ptr<base::TerminationSpace> Solved::clone() const {
	return std::make_unique<Solved>(*this);
}

bool Solved::is_done(scip::Model const& model) {
	return model.is_solved();
}

}  // namespace termination
}  // namespace ecole
