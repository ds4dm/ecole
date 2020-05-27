#pragma once

#include <memory>

#include <scip/scip.h>

#include "ecole/utility/reverse-control.hpp"

namespace ecole {
namespace scip {

struct ScipDeleter {
	void operator()(SCIP* ptr);
};

class Scimpl {
public:
	Scimpl();
	Scimpl(std::unique_ptr<SCIP, ScipDeleter>&&) noexcept;

	SCIP* get_scip_ptr() noexcept;

	Scimpl copy_orig();

	void solve_iter();
	void solve_iter_branch(SCIP_VAR* var);
	void solve_iter_stop();
	bool solve_iter_is_done();

private:
	std::unique_ptr<SCIP, ScipDeleter> m_scip = nullptr;
	std::unique_ptr<utility::Controller> m_controller = nullptr;
};

}  // namespace scip
}  // namespace ecole
