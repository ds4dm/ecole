#pragma once

#include <string>
#include <variant>

#include <scip/scip.h>

namespace ecole::scip {

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

using Param = std::variant<
	param_t<ParamType::Bool>,
	param_t<ParamType::Int>,
	param_t<ParamType::LongInt>,
	param_t<ParamType::Real>,
	param_t<ParamType::Char>,
	param_t<ParamType::String>>;

using Seed = int;
constexpr Seed min_seed = 1;  // 0 might be used for default.
constexpr Seed max_seed = 2147483647;

using Stage = SCIP_STAGE;
using Var = SCIP_VAR;
using Cons = SCIP_CONS;
using Col = SCIP_COL;
using Cons = SCIP_CONS;
using Row = SCIP_ROW;

/**
 * Class template to store the number of elements in Scip enums.
 *
 * Template is specialized for each Scip enum for which we need the number of element
 * (for instance for one-hot encoding categorical variables in observations).
 */
template <typename Enum> struct enum_size;

using base_stat = SCIP_BASESTAT;
template <> struct enum_size<base_stat> { static constexpr std::size_t value = 4; };

using var_type = SCIP_Vartype;
template <> struct enum_size<var_type> { static constexpr std::size_t value = 4; };

template <typename Enum> inline constexpr std::size_t enum_size_v = enum_size<Enum>::value;

}  // namespace ecole::scip
