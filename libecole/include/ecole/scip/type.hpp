#pragma once

#include <string>

#include <nonstd/variant.hpp>
#include <scip/scip.h>

namespace ecole {
namespace scip {

using nonstd::variant;

using long_int = SCIP_Longint;
using real = SCIP_Real;

/**
 * Types of parameters supported by SCIP.
 *
 * @see param_t to get the associated type.
 */
enum class ParamType { Bool, Int, LongInt, Real, Char, String };

namespace internal {
// Use with `param_t`.
template <ParamType> struct ParamType_get;
template <> struct ParamType_get<ParamType::Bool> { using type = bool; };
template <> struct ParamType_get<ParamType::Int> { using type = int; };
template <> struct ParamType_get<ParamType::LongInt> { using type = long_int; };
template <> struct ParamType_get<ParamType::Real> { using type = real; };
template <> struct ParamType_get<ParamType::Char> { using type = char; };
template <> struct ParamType_get<ParamType::String> { using type = std::string; };
}  // namespace internal

/**
 * Type associated with a ParamType.
 */
template <ParamType T> using param_t = typename internal::ParamType_get<T>::type;

using Param = nonstd::variant<
	param_t<ParamType::Bool>,
	param_t<ParamType::Int>,
	param_t<ParamType::LongInt>,
	param_t<ParamType::Real>,
	param_t<ParamType::Char>,
	param_t<ParamType::String>>;

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
