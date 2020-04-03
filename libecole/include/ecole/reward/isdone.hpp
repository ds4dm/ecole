#pragma once

#include <memory>

#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace reward {

class IsDone : public RewardFunction {
public:
	std::unique_ptr<RewardFunction> clone() const override;
	Reward get(environment::State const& state, bool done = false) override;
};

}  // namespace reward
}  // namespace ecole
