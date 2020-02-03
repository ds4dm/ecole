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

}  // namespace scip
}  // namespace ecole
