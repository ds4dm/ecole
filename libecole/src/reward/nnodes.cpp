#include "ecole/reward/nnodes.hpp"

#include "ecole/scip/model.hpp"

namespace ecole::reward {

void NNodes::before_reset(scip::Model& /* model */) {
	last_n_nodes = 0;
}

Reward NNodes::extract(scip::Model& model, bool /* done */) {
	auto n_nodes_diff = static_cast<std::uint64_t>(SCIPgetNTotalNodes(model.get_scip_ptr())) - last_n_nodes;
	last_n_nodes += n_nodes_diff;
	return static_cast<double>(n_nodes_diff);
}

}  // namespace ecole::reward
