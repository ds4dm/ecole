#pragma once

#include <cstdint>

#include "ecole/export.hpp"
#include "ecole/reward/abstract.hpp"
#include "scip/event_estim.h"
#include "scip/scip_event.h"

#define EVENTHDLR_NAME "estim"

namespace ecole::reward {

class ECOLE_EXPORT TreeSizeEstimate {
public:
	ECOLE_EXPORT auto before_reset(scip::Model& model) -> void;
	ECOLE_EXPORT auto extract(scip::Model& model, bool done = false) -> Reward;

private:
	SCIP_Real tree_size_estimate = 0.0;
};

}  // namespace ecole::reward
