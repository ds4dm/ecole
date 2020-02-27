#pragma once

#include <memory>

#include "ecole/environment/state.hpp"
#include "ecole/termination/abstract.hpp"

namespace ecole {
namespace termination {

class WhenSolved : public TerminationFunction<environment::State> {
public:
	using State = environment::State;

	std::unique_ptr<TerminationFunction> clone() const override;
	bool is_done(State const& state) override;
};

}  // namespace termination
}  // namespace ecole
