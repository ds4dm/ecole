#pragma once

namespace ecole::scip {
class Model;
}

namespace ecole::data {

/**
 * Abstract base class for all data extraction functions.
 *
 * An extraction function is given to environments to extract data, such as observations, rewards, and information.
 * It has a number of callbacks around dynamics transition to extract data as a function of the whole episode
 * history.
 *
 * @see observation::ObservationFunction
 * @see reward::RewardFunction
 */
template <typename Data> class DataFunction {
public:
	virtual ~DataFunction() = default;

	/**
	 * The method called by the environment on the initial state
	 *
	 * The method is called at the begining of every episode, and does nothing by default.
	 */
	virtual void before_reset(scip::Model& /* model */) {}

	/**
	 * The method called by environments when needing to extract data.
	 */
	virtual Data extract(scip::Model& model, bool done) = 0;
};

}  // namespace ecole::data
