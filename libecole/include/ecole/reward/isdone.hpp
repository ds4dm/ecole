#pragma once

#include <memory>

#include "ecole/environment/state.hpp"
#include "ecole/reward/abstract.hpp"

namespace ecole {
namespace reward {

class IsDone : public RewardFunction<environment::DefaultState> {
public:
	using State = environment::DefaultState;

	std::unique_ptr<RewardFunction> clone() const override;
	Reward get(State const& state, bool done = false) override;
};

}  // namespace reward
}  // namespace ecole
