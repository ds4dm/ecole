#pragma once

#include <scip/scip.h>

#include "ecole/scip/exception.hpp"

namespace ecole {
namespace scip {

Exception make_exception(SCIP_RETCODE retcode);

template <typename Func, typename... Arguments>
inline void call(Func func, Arguments&&... args) {
	auto retcode = func(std::forward<Arguments>(args)...);
	if (retcode != SCIP_OKAY) throw make_exception(retcode);
}

}  // namespace scip
}  // namespace ecole
