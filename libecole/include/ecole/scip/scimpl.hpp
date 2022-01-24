#pragma once

#include <memory>
#include <optional>
#include <utility>

#include <nonstd/span.hpp>
#include <scip/scip.h>

#include "ecole/export.hpp"
#include "ecole/scip/stop-location.hpp"

namespace ecole::utility {
template <typename Return, typename Message> class Coroutine;
}

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

	ECOLE_EXPORT auto solve_iter(nonstd::span<DynamicCallbackConstructor const> arg_packs) -> std::optional<Callback>;
	ECOLE_EXPORT auto solve_iter_continue(SCIP_RESULT result) -> std::optional<Callback>;

private:
	using Controller = utility::Coroutine<Callback, SCIP_RESULT>;

	std::unique_ptr<SCIP, ScipDeleter> m_scip;
	std::unique_ptr<Controller> m_controller;
};

}  // namespace ecole::scip
