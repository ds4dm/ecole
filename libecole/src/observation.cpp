#include "ecole/observation.hpp"

namespace ecole {

std::unique_ptr<Observation> BasicObs::Factory::make(scip::Model const& model) {
	(void)model;
	return std::make_unique<BasicObs>();
}

} // namespace ecole
