#pragma once

#include <memory>

namespace ecole {
namespace termination {

/**
 * Abstract base class for all termination functions.
 *
 * Termination functions can be given to environments to parametrize when the environment
 * terminates (that is, @ref Env::step returns `true` for the `done` flag).
 *
 * @tparam State The internal state used by the environment.
 */
template <typename State> class TerminationFunction {
public:
	virtual ~TerminationFunction() = default;
	virtual std::unique_ptr<TerminationFunction> clone() const = 0;

	/**
	 * The method called by the environment on the initial state
	 *
	 * The method is called at the begining of every episode, and does nothing by default.
	 */
	virtual void reset(State const& initial_state) { (void)initial_state; }

	/**
	 * The method called by the environment on every new state (after transitioning).
	 */
	virtual bool is_done(State const& state) = 0;
};

}  // namespace termination
}  // namespace ecole
