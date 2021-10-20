#include <scip/scip_general.h>

#include "ecole/version.hpp"

namespace ecole {

VersionInfo get_ecole_lib_version() noexcept {
	return get_ecole_header_version();
}

VersionInfo get_scip_lib_version() noexcept {
	return {
		static_cast<unsigned int>(SCIPmajorVersion()),
		static_cast<unsigned int>(SCIPminorVersion()),
		static_cast<unsigned int>(SCIPtechVersion()),
	};
}

VersionInfo get_scip_buildtime_version() noexcept {
	return get_scip_lib_version();
}

}  // namespace ecole
