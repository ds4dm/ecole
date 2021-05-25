#pragma once

#include <chrono>
#include <vector>

#include <scip/type_event.h>

#include "ecole/reward/abstract.hpp"
#include "ecole/reward/integral_eventhdlr.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::reward {

class PrimalIntegral : public RewardFunction {
public:
	PrimalIntegral(bool wall_ = false) noexcept : wall{wall_} {}
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	bool wall = false;
	SCIP_Real last_primal_integral;
	SCIP_Real initial_primal_bound;
	SCIP_Real primal_bound_reference;
};

}  // namespace ecole::reward
