#pragma once

#include <memory>

namespace ecole {
namespace observation {

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
 * @tparam State The internal state used by the environment.
 */
template <typename Observation, typename State> class ObservationFunction {
public:
	virtual ~ObservationFunction() = default;
	virtual std::unique_ptr<ObservationFunction> clone() const = 0;

	/**
	 * The method called by the environment on the initial state
	 *
	 * The method is called at the begining of every episode, and does nothing by default.
	 */
	virtual void reset(State const& initial_state) { (void)initial_state; }

	/**
	 * The method called by environments when needing to return an observation.
	 */
	virtual Observation get(State const& state) = 0;
};

}  // namespace observation
}  // namespace ecole
