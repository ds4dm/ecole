#pragma once

#include "ecole/export.hpp"
#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

class ECOLE_EXPORT IsDone {
public:
	auto before_reset(scip::Model& /*model*/) -> void {}
	ECOLE_EXPORT auto extract(scip::Model& model, bool done = false) -> Reward;
};

}  // namespace ecole::reward
