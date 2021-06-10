#pragma once

#include <functional>

#include "ecole/reward/abstract.hpp"
#include "ecole/scip/model.hpp"

namespace ecole::reward {

enum struct Bound {primal, dual, primal_dual};

template<Bound bound> class BoundIntegral : public RewardFunction {
public:
	using BoundFunction = std::function<std::tuple<SCIP_Real, SCIP_Real>(scip::Model& model)>;
	BoundIntegral(
		bool wall_ = false,
		BoundFunction bound_function_ = {}
	);
	void before_reset(scip::Model& model) override;
	Reward extract(scip::Model& model, bool done = false) override;

private:
	bool wall = false;
	BoundFunction bound_function;
	SCIP_Real last_integral = 0.0;
	SCIP_Real initial_primal_bound = 0.0;
	SCIP_Real initial_dual_bound = 0.0;
};

using PrimalIntegral = BoundIntegral<Bound::primal>; 
using DualIntegral = BoundIntegral<Bound::dual>; 
using PrimalDualIntegral = BoundIntegral<Bound::primal_dual>; 

}  // namespace ecole::reward
