#pragma once

#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace reward {

class Constant : public RewardFunction {
public:
	Reward const constant = 0.;

	Constant(Reward constant_ = 0.) : constant(constant_) {}

	Reward get(environment::State const&, bool) override { return constant; };
};

}  // namespace reward
}  // namespace ecole
