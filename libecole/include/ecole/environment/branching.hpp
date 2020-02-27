#pragma once

#include <cstddef>

#include "ecole/environment/default.hpp"
#include "ecole/environment/state.hpp"

namespace ecole {

namespace utility {
class Controller;
}

namespace environment {

template <typename... EnvTypes>
class Branching : public DefaultEnvironment<std::size_t, State, EnvTypes...> {
public:
	using Base = DefaultEnvironment<std::size_t, State, EnvTypes...>;

	using Base::Base;

	bool reset_state(State& initial_state) override;
	bool step_state(State& state, std::size_t const& action) override;

private:
	std::shared_ptr<utility::Controller> controller;
};

/*********************************
 *  Implementation of Branching  *
 *********************************/

namespace internal {
bool reset_state(std::shared_ptr<utility::Controller>&, State&);
bool step_state(std::shared_ptr<utility::Controller>&, State&, std::size_t const&);
}  // namespace internal

template <typename... EnvTypes>
bool Branching<EnvTypes...>::reset_state(State& init_state) {
	return internal::reset_state(controller, init_state);
}

template <typename... EnvTypes>
bool Branching<EnvTypes...>::step_state(State& state, std::size_t const& action) {
	return internal::step_state(controller, state, action);
}

}  // namespace environment
}  // namespace ecole
