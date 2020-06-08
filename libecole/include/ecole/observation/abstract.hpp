#pragma once

namespace ecole {

namespace scip {
class Model;
}

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
 */
template <typename Observation> class ObservationFunction {
public:
	virtual ~ObservationFunction() = default;

	/**
	 * The method called by the environment on the initial state
	 *
	 * The method is called at the begining of every episode, and does nothing by default.
	 */
	virtual void reset(scip::Model& /* model */) {}

	/**
	 * The method called by environments when needing to return an observation.
	 */
	virtual Observation obtain_observation(scip::Model& model) = 0;
};

}  // namespace observation
}  // namespace ecole
