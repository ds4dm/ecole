#include "ecole/version.hpp"

namespace ecole {

VersionInfo get_build_version() noexcept {
	return get_version();
}

}  // namespace ecole
