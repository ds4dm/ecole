#pragma once

#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class Constant : public RewardFunction {
public:
	Reward const constant = 0.;

	Constant(Reward constant_ = 0.) : constant(constant_) {}

	Reward obtain_reward(scip::Model const& /* model */, bool /* done */) override { return constant; };
};

}  // namespace ecole::reward
