#pragma once

#include <string>
#include <variant>

#include <scip/scip.h>

namespace ecole::scip {

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
template <> struct ParamType_get<ParamType::LongInt> { using type = SCIP_Longint; };
template <> struct ParamType_get<ParamType::Real> { using type = SCIP_Real; };
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

}  // namespace ecole::scip
