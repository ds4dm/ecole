#pragma once

#include <chrono>
#include <vector>

#include <scip/type_event.h>

#include "ecole/reward/abstract.hpp"
#include "ecole/reward/primaldualintegral_eventhdlr.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::reward {

class PrimalDualIntegral : public RewardFunction {
public:
	PrimalDualIntegral(bool wall_ = false) noexcept : wall{wall_} {}
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	bool wall = false;
	scip::real last_primal_dual_intgral;
	scip::real initial_primal_bound;
	scip::real initial_dual_bound;
	PrimalDualIntegralEventHandler* eventhdlr;
};

}  // namespace ecole::reward
