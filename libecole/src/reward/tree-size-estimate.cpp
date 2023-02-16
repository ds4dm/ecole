#include "ecole/reward/tree-size-estimate.hpp"

#include "ecole/scip/model.hpp"
#include "scip/def.h"

namespace ecole::reward {

void TreeSizeEstimate::before_reset(scip::Model& /* model */) {}

Reward TreeSizeEstimate::extract(scip::Model& model, bool /* done */) {
	// getTreeSizeEstimation returns -1 when no estimation has been made yet.
	tree_size_estimate = SCIPgetTreesizeEstimation(model.get_scip_ptr());
	return tree_size_estimate;
}

}  // namespace ecole::reward
