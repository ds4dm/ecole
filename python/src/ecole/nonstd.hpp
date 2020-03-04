#pragma once

#include <nonstd/variant.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace pybind11 {
namespace detail {

/**
 * Bind @ref nonstd::varaint for usage in Pybind.
 */
template <typename... Ts>
struct type_caster<nonstd::variant<Ts...>> : variant_caster<nonstd::variant<Ts...>> {};

/**
 * Specifies the function used to visit the variant.
 */
template <> struct visit_helper<nonstd::variant> {
	template <typename... Args>
	static auto call(Args&&... args) -> decltype(nonstd::visit(args...)) {
		return nonstd::visit(args...);
	}
};

}  // namespace detail
}  // namespace pybind11
