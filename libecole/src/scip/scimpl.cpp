#include <algorithm>
#include <cassert>
#include <mutex>
#include <utility>

#include <objscip/objbranchrule.h>
#include <objscip/objheur.h>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/scimpl.hpp"
#include "ecole/scip/utils.hpp"

namespace ecole::scip {

/******************************************
 *  Declaration of the ReverseBranchrule  *
 ******************************************/

namespace {

class ReverseBranchrule : public ::scip::ObjBranchrule {
public:
	static constexpr int max_priority = 536870911;
	static constexpr int no_maxdepth = -1;
	static constexpr double no_maxbounddist = 1.0;
	static constexpr auto name = "ecole::ReverseBranchrule";

	ReverseBranchrule(SCIP* scip, std::weak_ptr<utility::Controller::Executor> /*weak_executor_*/);

	auto scip_execlp(SCIP* scip, SCIP_BRANCHRULE* branchrule, SCIP_Bool allowaddcons, SCIP_RESULT* result)
		-> SCIP_RETCODE override;

private:
	std::weak_ptr<utility::Controller::Executor> weak_executor;
};

}  // namespace

/************************************
 *  Declaration of the ReverseHeur  *
 ************************************/

namespace {

class ReverseHeur : public ::scip::ObjHeur {
public:
	static constexpr int max_priority = 536870911;
	static constexpr auto name = "ecole::ReverseHeur";

	ReverseHeur(
		SCIP* scip,
		std::weak_ptr<utility::Controller::Executor> /*weak_executor*/,
		int trials_per_node,
		int depth_freq,
		int depth_start,
		int depth_stop);

	auto scip_exec(SCIP* scip, SCIP_HEUR* heur, SCIP_HEURTIMING heurtiming, SCIP_Bool nodeinfeasible, SCIP_RESULT* result)
		-> SCIP_RETCODE override;

private:
	std::weak_ptr<utility::Controller::Executor> weak_executor;

	int trials_per_node;
};

}  // namespace

/****************************
 *  Definition of Scimpl  *
 ****************************/

void ScipDeleter::operator()(SCIP* ptr) {
	scip::call(SCIPfree, &ptr);
}

namespace {

std::unique_ptr<SCIP, ScipDeleter> create_scip() {
	SCIP* scip_raw;
	scip::call(SCIPcreate, &scip_raw);
	SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip_raw), 1U);
	std::unique_ptr<SCIP, ScipDeleter> scip_ptr = nullptr;
	scip_ptr.reset(scip_raw);
	return scip_ptr;
}

}  // namespace

scip::Scimpl::Scimpl() : m_scip(create_scip()) {
	scip::call(SCIPincludeDefaultPlugins, get_scip_ptr());
}

Scimpl::Scimpl(std::unique_ptr<SCIP, ScipDeleter>&& scip_ptr) noexcept : m_scip(std::move(scip_ptr)) {}

SCIP* scip::Scimpl::get_scip_ptr() noexcept {
	return m_scip.get();
}

scip::Scimpl scip::Scimpl::copy() const {
	if (m_scip == nullptr) {
		return {nullptr};
	}
	if (SCIPgetStage(m_scip.get()) == SCIP_STAGE_INIT) {
		return {create_scip()};
	}
	auto dest = create_scip();
	// Copy operation is not thread safe
	static auto m = std::mutex{};
	auto g = std::lock_guard{m};
	scip::call(SCIPcopy, m_scip.get(), dest.get(), nullptr, nullptr, "", true, false, false, false, nullptr);
	return {std::move(dest)};
}

scip::Scimpl scip::Scimpl::copy_orig() const {
	if (m_scip == nullptr) {
		return {nullptr};
	}
	if (SCIPgetStage(m_scip.get()) == SCIP_STAGE_INIT) {
		return {create_scip()};
	}
	auto dest = create_scip();
	// Copy operation is not thread safe
	static auto m = std::mutex{};
	auto g = std::lock_guard{m};
	scip::call(SCIPcopyOrig, m_scip.get(), dest.get(), nullptr, nullptr, "", false, false, false, nullptr);
	return {std::move(dest)};
}

void Scimpl::solve_iter_start_branch() {
	auto* const scip_ptr = get_scip_ptr();
	m_controller =
		std::make_unique<utility::Controller>([scip_ptr](std::weak_ptr<utility::Controller::Executor> weak_executor) {
			scip::call(
				SCIPincludeObjBranchrule,
				scip_ptr,
				new ReverseBranchrule(scip_ptr, std::move(weak_executor)),  // NOLINT
				true);
			scip::call(SCIPsolve, scip_ptr);  // NOLINT
		});

	m_controller->wait_thread();
}

void scip::Scimpl::solve_iter_branch(SCIP_RESULT result) {
	m_controller->resume_thread([result](SCIP* /* scip */, SCIP_RESULT* final_result) {
		*final_result = result;
		return SCIP_OKAY;
	});
	m_controller->wait_thread();
}

void Scimpl::solve_iter_start_primalsearch(int trials_per_node, int depth_freq, int depth_start, int depth_stop) {
	auto* const scip_ptr = get_scip_ptr();
	m_controller = std::make_unique<utility::Controller>([=](std::weak_ptr<utility::Controller::Executor> weak_executor) {
		scip::call(
			SCIPincludeObjHeur,
			scip_ptr,
			new ReverseHeur(
				scip_ptr, std::move(weak_executor), trials_per_node, depth_freq, depth_start, depth_stop),  // NOLINT
			true);
		scip::call(SCIPsolve, scip_ptr);  // NOLINT
	});
	m_controller->wait_thread();
}

void scip::Scimpl::solve_iter_primalsearch(nonstd::span<std::pair<SCIP_VAR*, SCIP_Real>> const& varvals) {
	m_controller->resume_thread([&varvals](SCIP* scip_ptr, SCIP_RESULT* result) {
		SCIP_HEUR* heur = SCIPfindHeur(scip_ptr, scip::ReverseHeur::name);
		SCIP_Bool lperror = false;
		SCIP_Bool cutoff = false;
		SCIP_Bool success = false;

		// if action is empty, do nothing
		if (varvals.empty()) {
			*result = SCIP_DIDNOTFIND;
			return SCIP_OKAY;
		}

		// try to improve the (partial) solution by fixing variables and
		// then re-solving the LP

		// enter probing mode
		scip::call(SCIPstartProbing, scip_ptr);

		// fix the given variables to the given values
		for (auto const& [var, val] : varvals) {
			scip::call(SCIPfixVarProbing, scip_ptr, var, val);
		}

		// propagate
		scip::call(SCIPpropagateProbing, scip_ptr, 0, &cutoff, nullptr);
		if (!cutoff) {
			// build the LP if needed
			if (!SCIPisLPConstructed(scip_ptr)) {
				scip::call(SCIPconstructLP, scip_ptr, &cutoff);
			}
			if (!cutoff) {
				// solve the LP
				scip::call(SCIPsolveProbingLP, scip_ptr, -1, &lperror, &cutoff);
				if (!lperror && !cutoff) {
					// try the LP solution in the original problem
					SCIP_SOL* sol = nullptr;
					scip::call(SCIPcreateSol, scip_ptr, &sol, heur);
					scip::call(SCIPlinkLPSol, scip_ptr, sol);
					scip::call(SCIPtrySolFree, scip_ptr, &sol, false, true, true, true, true, &success);
				}
			}
		}

		// exit probing mode
		scip::call(SCIPendProbing, scip_ptr);

		*result = success ? SCIP_FOUNDSOL : SCIP_DIDNOTFIND;
		return SCIP_OKAY;
	});

	m_controller->wait_thread();
}

void scip::Scimpl::solve_iter_stop() {
	m_controller = nullptr;
}

bool scip::Scimpl::solve_iter_is_done() {
	return !(m_controller) || m_controller->is_done();
}

/*************************************
 *  Definition of ReverseBranchrule  *
 *************************************/

namespace {

scip::ReverseBranchrule::ReverseBranchrule(SCIP* scip, std::weak_ptr<utility::Controller::Executor> weak_executor_) :
	::scip::ObjBranchrule(
		scip,
		scip::ReverseBranchrule::name,
		"Branchrule that wait for another thread to make the branching.",
		scip::ReverseBranchrule::max_priority,
		scip::ReverseBranchrule::no_maxdepth,
		scip::ReverseBranchrule::no_maxbounddist),
	weak_executor(std::move(weak_executor_)) {}

auto ReverseBranchrule::scip_execlp(
	SCIP* scip,
	SCIP_BRANCHRULE* /*branchrule*/,
	SCIP_Bool /*allowaddcons*/,
	SCIP_RESULT* result) -> SCIP_RETCODE {
	if (weak_executor.expired()) {
		*result = SCIP_DIDNOTRUN;
		return SCIP_OKAY;
	}
	auto action_func = weak_executor.lock()->hold_env();
	return action_func(scip, result);
}

}  // namespace

/*******************************
 *  Definition of ReverseHeur  *
 *******************************/

namespace {

scip::ReverseHeur::ReverseHeur(
	SCIP* scip,
	std::weak_ptr<utility::Controller::Executor> weak_executor_,
	int trials_per_node_,
	int depth_freq,
	int depth_start,
	int depth_stop) :
	::scip::ObjHeur(
		scip,
		scip::ReverseHeur::name,
		"Primal heuristic that waits for another thread to provide a primal solution.",
		'e',
		scip::ReverseHeur::max_priority,
		depth_freq,
		depth_start,
		depth_stop,
		SCIP_HEURTIMING_AFTERNODE,
		false),
	weak_executor(std::move(weak_executor_)),
	trials_per_node(trials_per_node_) {}

auto ReverseHeur::scip_exec(
	SCIP* scip,
	SCIP_HEUR* /*heur*/,
	SCIP_HEURTIMING /*heurtiming*/,
	SCIP_Bool /*nodeinfeasible*/,
	SCIP_RESULT* result) -> SCIP_RETCODE {
	*result = SCIP_DIDNOTRUN;
	auto retcode = SCIP_OKAY;

	for (int trial = 0; trial < trials_per_node || trials_per_node < 0; trial++) {
		if (weak_executor.expired()) {
			return SCIP_OKAY;
		}

		auto action_func = weak_executor.lock()->hold_env();

		SCIP_RESULT action_result = SCIP_DIDNOTRUN;
		retcode = action_func(scip, &action_result);

		assert(action_result == SCIP_FOUNDSOL || action_result == SCIP_DIDNOTFIND || action_result == SCIP_DIDNOTRUN);

		// update primal heuristic result depending on search action result
		if (*result == SCIP_DIDNOTRUN || action_result == SCIP_FOUNDSOL) {
			*result = action_result;
		}

		// stop if anything went wrong, or if SCIP should be stopped
		if (retcode != SCIP_OKAY || action_result == SCIP_DIDNOTRUN || SCIPisStopped(scip)) {
			break;
		}
	}

	return retcode;
}

}  // namespace
}  // namespace ecole::scip
