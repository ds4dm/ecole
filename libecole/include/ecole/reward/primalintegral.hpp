#pragma once

#include <functional>

#include "ecole/reward/abstract.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::reward {

class PrimalIntegral : public RewardFunction {
public:
	PrimalIntegral(
		bool wall_ = false,
		std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)> bound_function_ =
			std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)>());
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	bool wall = false;
	std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)> bound_function;
	SCIP_Real last_primal_integral = 0.0;
	SCIP_Real initial_primal_bound = 0.0;
	SCIP_Real primal_bound_reference = 0.0;
};

}  // namespace ecole::reward
