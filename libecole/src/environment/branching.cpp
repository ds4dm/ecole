#include <memory>
#include <utility>

#include <objscip/objbranchrule.h>

#include "ecole/environment/branching.hpp"
#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"
#include "utility/reverse-control.hpp"

namespace ecole {
namespace environment {

namespace {

class ReverseBranchrule : public ::scip::ObjBranchrule {
public:
	static constexpr int max_priority = 536870911;
	static constexpr int no_maxdepth = -1;
	static constexpr double no_maxbounddist = 1.0;

	ReverseBranchrule(SCIP* scip, std::weak_ptr<utility::Controller> weak_controller);

	auto scip_execlp(
		SCIP* scip,
		SCIP_BRANCHRULE* branchrule,
		SCIP_Bool allowaddcons,
		SCIP_RESULT* result) -> SCIP_RETCODE override;

private:
	std::weak_ptr<utility::Controller> weak_controller;
};

ReverseBranchrule::ReverseBranchrule(
	SCIP* scip,
	std::weak_ptr<utility::Controller> weak_controller) :
	::scip::ObjBranchrule(
		scip,
		"ecole::ReverseBranchrule",
		"Branchrule that wait for another thread to make the branching.",
		max_priority,
		no_maxdepth,
		no_maxbounddist),
	weak_controller(weak_controller) {}

auto ReverseBranchrule::scip_execlp(
	SCIP* scip,
	SCIP_BRANCHRULE*,
	SCIP_Bool,
	SCIP_RESULT* result) -> SCIP_RETCODE {
	if (weak_controller.expired()) {
		*result = SCIP_DIDNOTRUN;
		return SCIP_OKAY;
	} else {
		auto controller = weak_controller.lock();
		auto action_func = controller->thread_interface().hold_env();
		return action_func(scip, result);
	}
}

}  // namespace

namespace internal {

bool reset_state(
	std::shared_ptr<utility::Controller>& controller,
	DefaultState& init_state) {
	auto& model = init_state.model;
	controller = utility::Controller::make_shared(
		[&model](std::weak_ptr<utility::Controller> weak_controller) {
			auto scip = model.get_scip_ptr();
			scip::call(
				SCIPincludeObjBranchrule,
				scip,
				new ReverseBranchrule(scip, weak_controller),
				true);
			model.solve();
		});

	controller->environment_interface().wait_thread();
	return controller->environment_interface().is_done();
}

static std::pair<SCIP_VAR**, std::size_t> lp_branch_cands(SCIP* scip) {
	SCIP_VAR** lp_cands = nullptr;
	int n_lp_cands = 0;
	scip::call(
		SCIPgetLPBranchCands,
		scip,
		&lp_cands,
		nullptr,
		nullptr,
		&n_lp_cands,
		nullptr,
		nullptr);
	return {lp_cands, n_lp_cands};
}

bool step_state(
	std::shared_ptr<utility::Controller>& controller,
	DefaultState&,
	std::size_t const& action) {
	controller->environment_interface().resume_thread(
		[action](SCIP* scip, SCIP_RESULT* result) {
			auto lp_cands = lp_branch_cands(scip);
			if (action >= lp_cands.second) return SCIP_ERROR;
			SCIP_CALL(SCIPbranchVar(scip, lp_cands.first[action], nullptr, nullptr, nullptr));
			*result = SCIP_BRANCHED;
			return SCIP_OKAY;
		});
	controller->environment_interface().wait_thread();
	return controller->environment_interface().is_done();
}

}  // namespace internal

}  // namespace environment
}  // namespace ecole
