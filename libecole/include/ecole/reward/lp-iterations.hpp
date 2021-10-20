#pragma once

#include <cstdint>

#include "ecole/export.hpp"
#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class ECOLE_EXPORT LpIterations : public RewardFunction {
public:
	ECOLE_EXPORT void before_reset(scip::Model& model) override;
	ECOLE_EXPORT Reward extract(scip::Model& model, bool done = false) override;

private:
	std::uint64_t last_lp_iter = 0;
};

}  // namespace ecole::reward
