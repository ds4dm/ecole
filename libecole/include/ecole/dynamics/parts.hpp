#pragma once

#include "ecole/export.hpp"
#include "ecole/random.hpp"
#include "ecole/scip/seed.hpp"

namespace ecole::scip {
class Model;
}

namespace ecole::dynamics {

/** Implementation of a default set_dynamics_random_state for dynamics classes. */
struct ECOLE_EXPORT DefaultSetDynamicsRandomState {

	/** Set random elements of the Model for the current episode. */
	ECOLE_EXPORT auto set_dynamics_random_state(scip::Model& model, RandomGenerator& rng) const -> void;
};

}  // namespace ecole::dynamics
