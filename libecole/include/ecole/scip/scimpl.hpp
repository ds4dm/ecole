#pragma once

#include <memory>
#include <optional>
#include <utility>

#include <nonstd/span.hpp>
#include <scip/scip.h>

#include "ecole/export.hpp"
#include "ecole/scip/stop-location.hpp"
#include "ecole/utility/coroutine.hpp"

namespace ecole::scip {

struct ECOLE_EXPORT ScipDeleter {
	ECOLE_EXPORT void operator()(SCIP* ptr);
};

class ECOLE_EXPORT Scimpl {
public:
	ECOLE_EXPORT Scimpl();
	ECOLE_EXPORT Scimpl(Scimpl&& /*other*/) noexcept;
	ECOLE_EXPORT Scimpl(std::unique_ptr<SCIP, ScipDeleter>&& /*scip_ptr*/) noexcept;
	ECOLE_EXPORT ~Scimpl();

	ECOLE_EXPORT auto get_scip_ptr() noexcept -> SCIP*;

	[[nodiscard]] ECOLE_EXPORT auto copy() const -> Scimpl;
	[[nodiscard]] ECOLE_EXPORT auto copy_orig() const -> Scimpl;

	template <typename... CallbackArgPacks> auto solve_iter(CallbackArgPacks... args) -> std::optional<Callback>;
	ECOLE_EXPORT auto solve_iter_continue(SCIP_RESULT result) -> std::optional<Callback>;

private:
	using Controller = utility::Coroutine<Callback, SCIP_RESULT>;
	using Executor = typename Controller::Executor;

	std::unique_ptr<SCIP, ScipDeleter> m_scip;
	std::unique_ptr<Controller> m_controller;

	template <Callback callback>
	static auto
	include_reverse_callback(SCIP* scip, std::weak_ptr<Executor> executor, CallbackConstructorArgs<callback> args)
		-> void;
};

}  // namespace ecole::scip

/**************************
 *  Definition of Scimpl  *
 **************************/

#include "ecole/scip/utils.hpp"

namespace ecole::scip {

template <typename... CallbackArgPacks> auto Scimpl::solve_iter(CallbackArgPacks... args) -> std::optional<Callback> {
	auto* const scip_ptr = get_scip_ptr();
	m_controller = std::make_unique<Controller>([=](std::weak_ptr<Controller::Executor> const& executor) {
		((Scimpl::include_reverse_callback(scip_ptr, executor, std::move(args))), ...);
		scip::call(SCIPsolve, scip_ptr);
	});
	return m_controller->wait();
}

}  // namespace ecole::scip
