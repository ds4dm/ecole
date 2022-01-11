#pragma once

#include <functional>
#include <string>

#include "ecole/export.hpp"
#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

enum struct ECOLE_EXPORT Bound { primal, dual, primal_dual };

template <Bound bound> class ECOLE_EXPORT BoundIntegral {
public:
	using BoundFunction = std::function<std::tuple<Reward, Reward>(scip::Model& model)>;

	ECOLE_EXPORT BoundIntegral(bool wall_ = false, const BoundFunction& bound_function_ = {});

	ECOLE_EXPORT auto before_reset(scip::Model& model) -> void;
	ECOLE_EXPORT auto extract(scip::Model& model, bool done = false) -> Reward;

private:
	BoundFunction bound_function;
	std::string name;
	Reward initial_primal_bound = 0.0;
	Reward initial_dual_bound = 0.0;
	Reward offset = 0.0;
	bool wall = false;
};

using PrimalIntegral = BoundIntegral<Bound::primal>;
using DualIntegral = BoundIntegral<Bound::dual>;
using PrimalDualIntegral = BoundIntegral<Bound::primal_dual>;

}  // namespace ecole::reward
