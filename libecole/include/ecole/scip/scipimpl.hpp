#pragma once

#include <memory>

#include <scip/scip.h>

namespace ecole {
namespace scip {

struct ScipDeleter {
	void operator()(SCIP* ptr);
};

class ScipImpl {
public:
	ScipImpl();
	ScipImpl(std::unique_ptr<SCIP, ScipDeleter>&&) noexcept;

	SCIP* get_scip_ptr() noexcept;

	ScipImpl copy_orig();

private:
	std::unique_ptr<SCIP, ScipDeleter> m_scip = nullptr;
};

}  // namespace scip
}  // namespace ecole
