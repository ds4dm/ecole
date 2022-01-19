#pragma once

#include <memory>
#include <optional>
#include <utility>

#include <nonstd/span.hpp>
#include <scip/scip.h>

#include "ecole/export.hpp"
#include "ecole/scip/stop-location.hpp"

namespace ecole {

namespace utility {
template <typename Yield, typename Message> class Coroutine;
}  // namespace utility

namespace scip {

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

	ECOLE_EXPORT auto solve_iter_start_branch() -> std::optional<StopLocation>;
	ECOLE_EXPORT auto solve_iter_branch(SCIP_RESULT result) -> std::optional<StopLocation>;

	ECOLE_EXPORT auto solve_iter_start_primalsearch(int depth_freq, int depth_start, int depth_stop)
		-> std::optional<StopLocation>;
	ECOLE_EXPORT auto solve_iter_primalsearch(SCIP_RESULT result) -> std::optional<StopLocation>;

private:
	using Controller = utility::Coroutine<StopLocation, SCIP_RESULT>;

	std::unique_ptr<SCIP, ScipDeleter> m_scip;
	std::unique_ptr<Controller> m_controller;
};

}  // namespace scip
}  // namespace ecole
