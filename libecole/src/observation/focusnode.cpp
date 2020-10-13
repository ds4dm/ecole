#include <array>
#include <cstddef>
#include <limits>

#include "ecole/observation/focusnode.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/type.hpp"

namespace ecole::observation {

std::optional<FocusNodeObs> FocusNode::obtain_observation(scip::Model& model) {

	if (model.get_stage() == SCIP_STAGE_SOLVING) {
		SCIP* scip = model.get_scip_ptr();
		SCIP_NODE* node = SCIPgetFocusNode(scip);

		long long number = SCIPnodeGetNumber(node) - 1;
		int depth = SCIPnodeGetDepth(node);
		double lowerbound = SCIPnodeGetLowerbound(node);
		double estimate = SCIPnodeGetEstimate(node);
		int n_added_conss = SCIPnodeGetNAddedConss(node);
		long long parent_number;
		double parent_lowerbound;

		if (number == 0) {
			// Root node
			parent_number = -1;
			parent_lowerbound = lowerbound;
		} else {
			SCIP_NODE* parent = SCIPnodeGetParent(node);
			parent_number = SCIPnodeGetNumber(parent) - 1;
			parent_lowerbound = SCIPnodeGetLowerbound(parent);
		}

		return FocusNodeObs{number, depth, lowerbound, estimate, n_added_conss, parent_number, parent_lowerbound};
	}
	return {};
}

}  // namespace ecole::observation
