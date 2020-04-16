#pragma once

#include <memory>

#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace reward {

class NegLPIterations : public RewardFunction {
public:
	std::unique_ptr<RewardFunction> clone() const override;
	void reset(environment::State const& initial_state) override;
	Reward get(environment::State const& state, bool done = false) override;

private:
	scip::long_int last_lp_iter;
};

}  // namespace reward
}  // namespace ecole
