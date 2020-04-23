#pragma once

#include <memory>

#include "ecole/environment/state.hpp"
#include "ecole/termination/abstract.hpp"

namespace ecole {
namespace termination {

class WhenSolved : public TerminationFunction {
public:
	bool is_done(environment::State const& state) override;
};

}  // namespace termination
}  // namespace ecole
