#pragma once

#include <memory>

#include "ecole/scip/model.hpp"

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
 */
template <typename Observation> class ObservationFunction {
public:
	using obs_t = Observation;

	virtual ~ObservationFunction() = default;
	virtual std::unique_ptr<ObservationFunction> clone() const = 0;

	/**
	 * The method called by the environment at the begining of every episode (on the
	 * initial state).
	 */
	virtual void reset(scip::Model const& model);

	/**
	 * The method called by environments when needing to return an observation.
	 */
	virtual obs_t get(scip::Model const& model) = 0;
};

/******************************************
 *  Implementation of ObservationFunction *
 ******************************************/

template <typename O> void ObservationFunction<O>::reset(scip::Model const& model) {
	(void)model;
}

}  // namespace observation
}  // namespace ecole
