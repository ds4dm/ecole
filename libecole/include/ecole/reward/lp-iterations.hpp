#pragma once

#include <cstdint>

#include "ecole/export.hpp"
#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class ECOLE_EXPORT LpIterations {
public:
	ECOLE_EXPORT auto before_reset(scip::Model& model) -> void;
	ECOLE_EXPORT auto extract(scip::Model& model, bool done = false) -> Reward;

private:
	std::uint64_t last_lp_iter = 0;
};

}  // namespace ecole::reward
