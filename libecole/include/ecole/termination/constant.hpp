#pragma once

#include "ecole/termination/abstract.hpp"

namespace ecole {
namespace termination {

class Constant : public TerminationFunction {
public:
	bool const constant = false;

	Constant(bool constant_ = false) : constant(constant_) {}

	bool obtain_termination(environment::State const&) override { return constant; };
};

}  // namespace termination
}  // namespace ecole
