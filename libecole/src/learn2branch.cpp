#include <scip/scip.h>

#include "ecole/learn2branch.hpp"
#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"

namespace ecole {

BranchEnv::BranchEnv(scip::Model&& other_model) noexcept :
	model(std::move(other_model)) {}

void BranchEnv::run(scip::BranchFunc const& func) {
	model.set_branch_rule(func);
	model.solve();
}

} // namespace ecole
