#pragma once

#include <memory>

#include <nonstd/span.hpp>
#include <scip/scip.h>

#include "ecole/utility/reverse-control.hpp"

namespace ecole::scip {

struct ScipDeleter {
	void operator()(SCIP* ptr);
};

class Scimpl {
public:
	Scimpl();
	Scimpl(std::unique_ptr<SCIP, ScipDeleter>&& /*scip_ptr*/) noexcept;

	SCIP* get_scip_ptr() noexcept;

	[[nodiscard]] Scimpl copy() const;
	[[nodiscard]] Scimpl copy_orig() const;

	void solve_iter();
	void solve_iter_branch(SCIP_RESULT result);
	void solve_iter_stop();
	bool solve_iter_is_done();

private:
	std::unique_ptr<SCIP, ScipDeleter> m_scip = nullptr;
	std::unique_ptr<utility::Controller> m_controller = nullptr;
};

}  // namespace ecole::scip
