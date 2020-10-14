#include <mutex>

#include <objscip/objbranchrule.h>
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

	ReverseBranchrule(SCIP* scip, std::weak_ptr<utility::Controller::Executor> /*weak_executor_*/);

	auto scip_execlp(SCIP* scip, SCIP_BRANCHRULE* branchrule, SCIP_Bool allowaddcons, SCIP_RESULT* result)
		-> SCIP_RETCODE override;

private:
	std::weak_ptr<utility::Controller::Executor> weak_executor;
};

}  // namespace

/****************************
 *  Definition of Scimpl  *
 ****************************/

void ScipDeleter::operator()(SCIP* ptr) {
	scip::call(SCIPfree, &ptr);
}

static std::unique_ptr<SCIP, ScipDeleter> create_scip() {
	SCIP* scip_raw;
	scip::call(SCIPcreate, &scip_raw);
	SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip_raw), 1U);
	std::unique_ptr<SCIP, ScipDeleter> scip_ptr = nullptr;
	scip_ptr.reset(scip_raw);
	return scip_ptr;
}

static std::unique_ptr<SCIP, ScipDeleter> copy_orig(SCIP const* const source) {
	if (source == nullptr) {
		return nullptr;
	}
	if (SCIPgetStage(const_cast<SCIP*>(source)) == SCIP_STAGE_INIT) {
		return create_scip();
	}
	auto dest = create_scip();
	// Copy operation is not thread safe
	static std::mutex m{};
	std::lock_guard<std::mutex> g{m};
	scip::call(SCIPcopyOrig, const_cast<SCIP*>(source), dest.get(), nullptr, nullptr, "", false, false, false, nullptr);
	return dest;
}

scip::Scimpl::Scimpl() : m_scip(create_scip()) {
	scip::call(SCIPincludeDefaultPlugins, get_scip_ptr());
}

Scimpl::Scimpl(std::unique_ptr<SCIP, ScipDeleter>&& scip_ptr) noexcept : m_scip(std::move(scip_ptr)) {}

SCIP* scip::Scimpl::get_scip_ptr() noexcept {
	return m_scip.get();
}

scip::Scimpl scip::Scimpl::copy_orig() {
	return ::ecole::scip::copy_orig(get_scip_ptr());
}

void Scimpl::solve_iter() {
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

void scip::Scimpl::solve_iter_branch(SCIP_VAR* var) {
	m_controller->resume_thread([var](SCIP* scip_ptr, SCIP_RESULT* result) {
		if (var == nullptr) {
			*result = SCIP_DIDNOTRUN;
		} else {
			SCIP_CALL(SCIPbranchVar(scip_ptr, var, nullptr, nullptr, nullptr));
			*result = SCIP_BRANCHED;
		}
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
		"ecole::ReverseBranchrule",
		"Branchrule that wait for another thread to make the branching.",
		scip::ReverseBranchrule::max_priority,
		scip::ReverseBranchrule::no_maxdepth,
		no_maxbounddist),
	weak_executor(std::move(weak_executor_)) {}

auto ReverseBranchrule::scip_execlp(SCIP* scip, SCIP_BRANCHRULE* /*branchrule*/, SCIP_Bool, SCIP_RESULT* result)
	-> SCIP_RETCODE {
	if (weak_executor.expired()) {
		*result = SCIP_DIDNOTRUN;
		return SCIP_OKAY;
	}
	auto action_func = weak_executor.lock()->hold_env();
	return action_func(scip, result);
}

}  // namespace
}  // namespace ecole::scip
