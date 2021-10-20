#pragma once

#include "ecole/export.hpp"
#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class ECOLE_EXPORT IsDone : public RewardFunction {
public:
	ECOLE_EXPORT Reward extract(scip::Model& model, bool done = false) override;
};

}  // namespace ecole::reward
