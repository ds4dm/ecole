#pragma once

#include <scip/scip.h>

namespace ecole {
namespace scip {

using long_int = SCIP_Longint;
using real = SCIP_Real;

/**
 * Class template to store the number of elements in Scip enums.
 *
 * Template is specialized for each Scip enum for which we need the number of element
 * (for instance for one-hot encoding categorical variables in observations).
 */
template <typename Enum> struct enum_size;

using Seed = int;
constexpr Seed min_seed = 0;
constexpr Seed max_seed = 2147483647;

}  // namespace scip
}  // namespace ecole
