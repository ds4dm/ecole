#pragma once

#include "ecole/reward/abstract.hpp"
#include "ecole/scip/type.hpp"

#define infinity   1e+20

namespace ecole::reward {

class DualBound : public RewardFunction {
public:
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
//	scip::real dual_bound_value = 0.0;
	scip::real dual_bound_value = - infinity;
};

}  // namespace ecole::reward
