#pragma once

#include <scip/scip.h>

#include "ecole/scip/exception.hpp"

namespace ecole::scip {

template <typename Func, typename... Arguments> inline void call(Func func, Arguments&&... args) {
	scip::ScipError::reset_message_capture();
	auto retcode = func(std::forward<Arguments>(args)...);
	if (retcode != SCIP_OKAY) {
		throw scip::ScipError::from_retcode(retcode);
	}
}

}  // namespace ecole::scip
