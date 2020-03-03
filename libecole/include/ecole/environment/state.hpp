#pragma once

#include <ecole/scip/model.hpp>

namespace ecole {
namespace environment {

/**
 * A minimal state class that contains only a @ref scip::Model.
 *
 * Represent the state of an Markov Decision Process, and is passed along to extract
 * observations, rewards, etc.
 * Can safely be inherited from to defined mode specialized states.
 */
class State {
public:
	scip::Model model;

	State() = default;
	explicit State(scip::Model&& model) : model(std::move(model)){};
	explicit State(scip::Model const& model) : model(model){};
	State(State const&) = default;
	State(State&&) = default;
	State& operator=(State const&) = default;
	State& operator=(State&&) = default;
	virtual ~State() = default;
};

}  // namespace environment
}  // namespace ecole
