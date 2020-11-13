#pragma once

#include "ecole/data/abstract.hpp"

namespace ecole::observation {

/**
 * Abstract base class for all observation functions.
 *
 * Observation functions can be given to environments to parametrize what observations
 * (or partially observed states) are returned at every transition.
 * An observation function is intended to extract the observation out of the scip::Model
 * in any way desired (including caching, scaling...).
 * An observation on the contrary hand is a purely self contained data class with no
 * function.
 *
 * @tparam Observation the type of the observation extracted by this class.
 */
template <typename Observation> using ObservationFunction = data::DataFunction<Observation>;

}  // namespace ecole::observation
