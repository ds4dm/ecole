#pragma once

#include <memory>
#include <utility>

#include <nonstd/span.hpp>
#include <scip/scip.h>

#include "ecole/export.hpp"

namespace ecole {

namespace utility {
class Controller;
}

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

	ECOLE_EXPORT SCIP* get_scip_ptr() noexcept;

	[[nodiscard]] ECOLE_EXPORT Scimpl copy() const;
	[[nodiscard]] ECOLE_EXPORT Scimpl copy_orig() const;

	ECOLE_EXPORT void solve_iter_start_branch();
	ECOLE_EXPORT void solve_iter_branch(SCIP_RESULT result);
	ECOLE_EXPORT SCIP_HEUR*
	solve_iter_start_primalsearch(int trials_per_node, int depth_freq, int depth_start, int depth_stop);
	ECOLE_EXPORT void solve_iter_primalsearch(SCIP_RESULT result);
	ECOLE_EXPORT void solve_iter_stop();
	ECOLE_EXPORT bool solve_iter_is_done();

private:
	std::unique_ptr<SCIP, ScipDeleter> m_scip;
	std::unique_ptr<utility::Controller> m_controller;
};

}  // namespace scip
}  // namespace ecole
