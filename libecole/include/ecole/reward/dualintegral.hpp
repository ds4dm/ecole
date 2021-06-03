#pragma once

#include <chrono>
#include <vector>
#include <functional>

#include <scip/type_event.h>

#include "ecole/reward/abstract.hpp"
#include "ecole/reward/integral_eventhdlr.hpp"
#include "ecole/scip/type.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::reward {

class DualIntegral : public RewardFunction {
public:
	DualIntegral(
		bool wall_ = false, 
		std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)> bound_function_ = std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)>()
	);
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	bool wall = false;
	std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)> bound_function;
	SCIP_Real last_dual_integral = 0.0;
	SCIP_Real initial_dual_bound = 0.0;
	SCIP_Real dual_bound_reference = 0.0;
};

}  // namespace ecole::reward
