#pragma once

#include <memory>

#include "ecole/reward/abstract.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace reward {

class IsDone : public RewardFunction {
public:
	std::unique_ptr<RewardFunction> clone() const override;
	Reward get(scip::Model const& model, bool done = false) override;
};

}  // namespace reward
}  // namespace ecole
