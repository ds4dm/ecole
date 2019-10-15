#include <scip/scip.h>

#include "ecole/learn2branch.hpp"

namespace ecole {

static void run_trajectory(
	scip::Model model,
	Observation::Factory* const factory,
	BranchEnv::BranchFunc const& branch_func) {

	auto const branch_rule = [branch_func, factory](scip::Model const& model) {
		auto obs = factory->make(model);
		auto const var_idx = branch_func(std::move(obs));
		return model.lp_branch_vars()[var_idx];
	};
	model.set_branch_rule(branch_rule);
	model.solve();
}

BranchEnv::BranchEnv(scip::Model&& model, std::unique_ptr<ObsFactory> factory) noexcept :
	model(std::move(model)), factory(std::move(factory)) {}

void BranchEnv::run(BranchFunc const& func) {
	run_trajectory(model, factory.get(), func);
}

} // namespace ecole
