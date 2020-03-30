#pragma once

#include <memory>

#include "ecole/reward/abstract.hpp"
#include "ecole/termination/abstract.hpp"

namespace ecole {
namespace reward {

class NLPIterations : public RewardFunction {
	SCIP_Longint counter;

public:
	std::unique_ptr<RewardFunction> clone() const override;
	void reset(environment::State const& initial_state) override;
	Reward get(environment::State const& state, bool done = false) override;
};

}  // namespace reward
}  // namespace ecole
