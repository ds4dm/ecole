#include "ecole/reward/nnodes.hpp"

#include "ecole/scip/model.hpp"

namespace ecole {
namespace reward {

void NNodes::reset(scip::Model const&) {
	last_n_nodes = 0;
}

Reward NNodes::obtain_reward(scip::Model const& model, bool /* done */) {
	auto n_nodes_diff = SCIPgetNTotalNodes(model.get_scip_ptr()) - last_n_nodes;
	last_n_nodes += n_nodes_diff;
	return static_cast<double>(-n_nodes_diff);
}

}  // namespace reward
}  // namespace ecole
