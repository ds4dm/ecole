#pragma once

namespace ecole {

namespace scip {
class Model;
}

namespace termination {

/**
 * Abstract base class for all termination functions.
 *
 * Termination functions can be given to environments to parametrize when the environment
 * terminates (that is, @ref environment::Environment::step returns `true` for the `done`
 * flag).
 */
class TerminationFunction {
public:
	virtual ~TerminationFunction() = default;

	/**
	 * The method called by the environment on the initial state
	 *
	 * The method is called at the begining of every episode, and does nothing by default.
	 */
	virtual void reset(scip::Model const&) {}

	/**
	 * The method called by the environment on every new state (after transitioning).
	 */
	virtual bool obtain_termination(scip::Model const&) = 0;
};

}  // namespace termination
}  // namespace ecole
