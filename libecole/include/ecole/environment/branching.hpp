#pragma once

#include <cstddef>

#include "ecole/environment/default.hpp"
#include "ecole/environment/state.hpp"

namespace ecole {
namespace environment {

template <typename... EnvTypes>
class Branching : public DefaultEnvironment<std::size_t, DefaultState, EnvTypes...> {
public:
	using Base = DefaultEnvironment<std::size_t, DefaultState, EnvTypes...>;

	using Base::Base;

	bool reset_state(DefaultState& initial_state) override;
	bool step_state(DefaultState& state, std::size_t const& action) override;
};

/*********************************
 *  Implementation of Branching  *
 *********************************/

template <typename... EnvTypes> bool Branching<EnvTypes...>::reset_state(DefaultState&) {
	return false;
}

template <typename... EnvTypes>
bool Branching<EnvTypes...>::step_state(DefaultState&, std::size_t const&) {
	return true;
}

}  // namespace environment
}  // namespace ecole
