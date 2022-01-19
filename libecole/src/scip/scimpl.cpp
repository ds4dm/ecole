#include <algorithm>
#include <cassert>
#include <mutex>
#include <scip/type_result.h>
#include <scip/type_retcode.h>
#include <type_traits>
#include <utility>

#include <objscip/objbranchrule.h>
#include <objscip/objheur.h>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/scimpl.hpp"
#include "ecole/scip/stop-location.hpp"
#include "ecole/scip/utils.hpp"
#include "ecole/utility/coroutine.hpp"

namespace ecole::scip {

namespace {

/******************************************
 *  Declaration of the ReverseBranchrule  *
 ******************************************/

using Controller = utility::Coroutine<StopLocation, SCIP_RESULT>;

class ReverseBranchrule : public ::scip::ObjBranchrule {
public:
	static constexpr int max_priority = 536870911;
	static constexpr int no_maxdepth = -1;
	static constexpr double no_maxbounddist = 1.0;
	static constexpr auto name = callback_name(StopLocation::Branchrule);

	ReverseBranchrule(SCIP* scip, std::weak_ptr<Controller::Executor> /*weak_executor_*/);

	auto scip_execlp(SCIP* scip, SCIP_BRANCHRULE* branchrule, SCIP_Bool allowaddcons, SCIP_RESULT* result)
		-> SCIP_RETCODE override;

private:
	std::weak_ptr<Controller::Executor> weak_executor;
};

/************************************
 *  Declaration of the ReverseHeur  *
 ************************************/

class ReverseHeur : public ::scip::ObjHeur {
public:
	static constexpr int max_priority = 536870911;
	static constexpr auto name = callback_name(StopLocation::Heurisitc);

	ReverseHeur(
		SCIP* scip,
		std::weak_ptr<Controller::Executor> /*weak_executor*/,
		int depth_freq,
		int depth_start,
		int depth_stop);

	auto scip_exec(SCIP* scip, SCIP_HEUR* heur, SCIP_HEURTIMING heurtiming, SCIP_Bool nodeinfeasible, SCIP_RESULT* result)
		-> SCIP_RETCODE override;

private:
	std::weak_ptr<Controller::Executor> weak_executor;
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
	std::unique_ptr<SCIP, ScipDeleter> scip_ptr = nullptr;
	scip_ptr.reset(scip_raw);
	return scip_ptr;
}

}  // namespace

Scimpl::Scimpl() : m_scip{create_scip()} {}

Scimpl::Scimpl(Scimpl&&) noexcept = default;

Scimpl::Scimpl(std::unique_ptr<SCIP, ScipDeleter>&& scip_ptr) noexcept : m_scip(std::move(scip_ptr)) {}

Scimpl::~Scimpl() = default;

auto Scimpl::get_scip_ptr() noexcept -> SCIP* {
	return m_scip.get();
}

auto Scimpl::copy() const -> Scimpl {
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

auto Scimpl::copy_orig() const -> Scimpl {
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

auto Scimpl::solve_iter_start_branch() -> std::optional<StopLocation> {
	auto* const scip_ptr = get_scip_ptr();
	m_controller = std::make_unique<Controller>([scip_ptr](std::weak_ptr<Controller::Executor> weak_executor) {
		scip::call(
			SCIPincludeObjBranchrule,
			scip_ptr,
			new ReverseBranchrule(scip_ptr, std::move(weak_executor)),  // NOLINT
			true);
		scip::call(SCIPsolve, scip_ptr);  // NOLINT
	});

	return m_controller->wait();
}

auto scip::Scimpl::solve_iter_branch(SCIP_RESULT result) -> std::optional<StopLocation> {
	m_controller->resume(result);
	return m_controller->wait();
}

auto Scimpl::solve_iter_start_primalsearch(int depth_freq, int depth_start, int depth_stop)
	-> std::optional<StopLocation> {
	auto* const scip_ptr = get_scip_ptr();
	m_controller = std::make_unique<Controller>([=](std::weak_ptr<Controller::Executor> weak_executor) {
		scip::call(
			SCIPincludeObjHeur,
			scip_ptr,
			new ReverseHeur(scip_ptr, std::move(weak_executor), depth_freq, depth_start, depth_stop),  // NOLINT
			true);
		scip::call(SCIPsolve, scip_ptr);  // NOLINT
	});

	return m_controller->wait();
}

auto scip::Scimpl::solve_iter_primalsearch(SCIP_RESULT result) -> std::optional<StopLocation> {
	m_controller->resume(result);
	return m_controller->wait();
}

namespace {

/*************************************
 *  Definition of ReverseBranchrule  *
 *************************************/

template <StopLocation Loc>
auto handle_executor(std::weak_ptr<Controller::Executor>& weak_executor, SCIP* scip, SCIP_RESULT* result)
	-> SCIP_RETCODE {
	if (weak_executor.expired()) {
		*result = SCIP_DIDNOTRUN;
		return SCIP_OKAY;
	}
	return std::visit(
		[&](auto result_or_stop) -> SCIP_RETCODE {
			using StopToken = Controller::Executor::StopToken;
			if constexpr (std::is_same_v<decltype(result_or_stop), StopToken>) {
				*result = SCIP_DIDNOTRUN;
				return SCIPinterruptSolve(scip);
			} else {
				*result = result_or_stop;
				return SCIP_OKAY;
			}
		},
		weak_executor.lock()->yield(Loc));
}

scip::ReverseBranchrule::ReverseBranchrule(SCIP* scip, std::weak_ptr<Controller::Executor> weak_executor_) :
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
	return handle_executor<StopLocation::Branchrule>(weak_executor, scip, result);
}

/*******************************
 *  Definition of ReverseHeur  *
 *******************************/

scip::ReverseHeur::ReverseHeur(
	SCIP* scip,
	std::weak_ptr<Controller::Executor> weak_executor_,
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
	weak_executor(std::move(weak_executor_)) {}

auto ReverseHeur::scip_exec(
	SCIP* scip,
	SCIP_HEUR* /*heur*/,
	SCIP_HEURTIMING /*heurtiming*/,
	SCIP_Bool /*nodeinfeasible*/,
	SCIP_RESULT* result) -> SCIP_RETCODE {
	return handle_executor<StopLocation::Heurisitc>(weak_executor, scip, result);
}

}  // namespace
}  // namespace ecole::scip
