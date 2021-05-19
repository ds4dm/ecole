#pragma once

#include <chrono>
#include <vector>

#include <scip/type_event.h>

#include "ecole/reward/abstract.hpp"
#include "ecole/reward/dualintegral_eventhdlr.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::reward {

class DualIntegral : public RewardFunction {
public:
	DualIntegral(bool wall_ = false) noexcept : wall{wall_} {}
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	bool wall = false;
	scip::real last_dual_intgral;
	scip::real initial_dual_bound;
	scip::real dual_bound_reference;
	DualIntegralEventHandler* eventhdlr;
};

}  // namespace ecole::reward
