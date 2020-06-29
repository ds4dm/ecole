#include "ecole/version.hpp"

namespace ecole {

constexpr VersionInfo get_lib_version() noexcept {
	return get_version();
}

}  // namespace ecole
