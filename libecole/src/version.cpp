#include <dlfcn.h>
#include <stdexcept>

#include <scip/scip_general.h>

#include "ecole/version.hpp"

namespace ecole::version {

auto get_ecole_lib_version() noexcept -> VersionInfo {
	return get_ecole_header_version();
}

/**
 * Non-standard way to extract the path of the current library (libecole).
 *
 * Inspired by:
 *  - https://stackoverflow.com/a/57201397/5862073
 *  - https://github.com/gpakosz/whereami
 * On Windows, see:
 *  - https://stackoverflow.com/a/55983632/5862073
 */
auto get_ecole_lib_path() -> std::filesystem::path {
	if (Dl_info info; dladdr(reinterpret_cast<void const*>(get_ecole_lib_path), &info)) {
		return info.dli_fname;
	}
	throw std::runtime_error{"Cannot find path of Ecole library"};
}

auto get_scip_lib_version() noexcept -> VersionInfo {
	return {
		static_cast<unsigned int>(SCIPmajorVersion()),
		static_cast<unsigned int>(SCIPminorVersion()),
		static_cast<unsigned int>(SCIPtechVersion()),
	};
}

auto get_scip_buildtime_version() noexcept -> VersionInfo {
	return get_scip_lib_version();
}

}  // namespace ecole::version
