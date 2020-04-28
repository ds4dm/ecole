#include <memory>
#include <utility>

#include <nonstd/span.hpp>
#include <objscip/objbranchrule.h>

#include "ecole/environment/branching.hpp"
#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"

namespace ecole {
namespace environment {

/***************************************
 *  Definition of ReverseControlState  *
 ***************************************/

ReverseControlState::ReverseControlState(scip::Model&& p_model) :
	State(std::move(p_model)) {}

ReverseControlState::ReverseControlState(scip::Model const& p_model) : State(p_model) {}

ReverseControlState::ReverseControlState(ReverseControlState&& rc_state) {
	controller = std::move(rc_state.controller);
	this->model = std::move(rc_state.model);
}

ReverseControlState& ReverseControlState::operator=(ReverseControlState&& rc_state) {
	controller = std::move(rc_state.controller);
	State::operator=(std::move(rc_state));
	return *this;
}

/******************************************
 *  Declaration of the ReverseBranchrule  *
 ******************************************/

namespace {

class ReverseBranchrule : public ::scip::ObjBranchrule {
public:
	static constexpr int max_priority = 536870911;
	static constexpr int no_maxdepth = -1;
	static constexpr double no_maxbounddist = 1.0;

	ReverseBranchrule(SCIP* scip, std::weak_ptr<utility::Controller::Executor>);

	auto scip_execlp(
		SCIP* scip,
		SCIP_BRANCHRULE* branchrule,
		SCIP_Bool allowaddcons,
		SCIP_RESULT* result) -> SCIP_RETCODE override;

private:
	std::weak_ptr<utility::Controller::Executor> weak_executor;
};

}  // namespace

/*************************************
 *  Definition of BranchingDynamics  *
 *************************************/

std::tuple<bool, xt::xtensor<std::size_t, 1>>
BranchingDynamics::reset_dynamics(State& init_state) {
	auto& model = init_state.model;
	init_state.controller = std::make_unique<utility::Controller>(
		[&model](std::weak_ptr<utility::Controller::Executor> weak_executor) {
			auto scip = model.get_scip_ptr();
			scip::call(
				SCIPincludeObjBranchrule,
				scip,
				new ReverseBranchrule(scip, weak_executor),  // NOLINT
				true);
			model.solve();  // NOLINT
		});

	init_state.controller->wait_thread();
	return {init_state.controller->is_done(), {}};
}

static nonstd::span<SCIP_VAR*> lp_branch_cands(SCIP* scip) {
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
	assert(n_lp_cands >= 0);
	return {lp_cands, static_cast<std::size_t>(n_lp_cands)};
}

std::tuple<bool, xt::xtensor<std::size_t, 1>>
BranchingDynamics::step_dynamics(State& state, std::size_t const& action) {
	state.controller->resume_thread([action](SCIP* scip, SCIP_RESULT* result) {
		auto const lp_cands = lp_branch_cands(scip);
		if (action >= lp_cands.size()) return SCIP_ERROR;
		SCIP_CALL(SCIPbranchVar(scip, lp_cands[action], nullptr, nullptr, nullptr));
		*result = SCIP_BRANCHED;
		return SCIP_OKAY;
	});
	state.controller->wait_thread();
	return {state.controller->is_done(), {}};
}

/*************************************
 *  Definition of ReverseBranchrule  *
 *************************************/

namespace {

ReverseBranchrule::ReverseBranchrule(
	SCIP* scip,
	std::weak_ptr<utility::Controller::Executor> weak_executor_) :
	::scip::ObjBranchrule(
		scip,
		"ecole::ReverseBranchrule",
		"Branchrule that wait for another thread to make the branching.",
		max_priority,
		no_maxdepth,
		no_maxbounddist),
	weak_executor(weak_executor_) {}

auto ReverseBranchrule::scip_execlp(
	SCIP* scip,
	SCIP_BRANCHRULE*,
	SCIP_Bool,
	SCIP_RESULT* result) -> SCIP_RETCODE {
	if (weak_executor.expired()) {
		*result = SCIP_DIDNOTRUN;
		return SCIP_OKAY;
	} else {
		auto action_func = weak_executor.lock()->hold_env();
		return action_func(scip, result);
	}
}

}  // namespace

}  // namespace environment
}  // namespace ecole
