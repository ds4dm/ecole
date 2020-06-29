#include "ecole/version.hpp"

namespace ecole {

VersionInfo get_build_version() noexcept {
	return get_version();
}

VersionInfo get_build_scip_version() noexcept {
	return get_scip_version();
}

}  // namespace ecole
