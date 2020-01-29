#pragma once

#include <scip/scip.h>

namespace ecole {
namespace scip {

using long_int = SCIP_Longint;
using real = SCIP_Real;

template <typename Enum> struct enum_size;

using var_type = SCIP_Vartype;
template <> struct enum_size<var_type> { static constexpr std::size_t value = 4; };

}  // namespace scip
}  // namespace ecole
