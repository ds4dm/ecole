#pragma once

#include <chrono>
#include <vector>

#include <scip/type_event.h>

#include "ecole/reward/primalintegral_eventhdlr.hpp"
#include "ecole/reward/abstract.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::reward {

class PrimalIntegral : public RewardFunction {
public:
	PrimalIntegral(bool wall_ = false) noexcept : wall{wall_} {}
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	bool wall = false;
	scip::real last_primal_intgral;
	scip::real initial_primal_bound;
	scip::real primal_bound_reference;
	PrimalIntegralEventHandler* eventhdlr;
};

}  // namespace ecole::reward