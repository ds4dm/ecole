#include "ecole/env/observation.hpp"

namespace ecole {
namespace env {

std::unique_ptr<Observation> BasicObs::Factory::make(scip::Model const& model) {
	(void)model;
	return std::make_unique<BasicObs>();
}

} // namespace env
} // namespace ecole
