#pragma once

#include <memory>

#include "ecole/scip/model.hpp"

namespace ecole {
namespace termination {

/**
 * Abstract base class for all termination functions.
 *
 * Termination functions can be given to environments to parametrize when the environment
 * terminates (that is, @ref Env::step returns `true` for the `done` flag).
 */
class TerminationFunction {
public:
	virtual ~TerminationFunction() = default;
	virtual std::unique_ptr<TerminationFunction> clone() const = 0;

	/**
	 * The method called by the environment at the begining of every episode (on the
	 * initial state).
	 */
	virtual void reset(scip::Model const& model);

	/**
	 * The method called by the environment on every new state (after transitioning).
	 */
	virtual bool is_done(scip::Model const& model) = 0;
};

}  // namespace termination
}  // namespace ecole
