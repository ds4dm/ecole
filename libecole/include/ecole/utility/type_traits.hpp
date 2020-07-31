#pragma once

#include <type_traits>

namespace ecole {
namespace utility {

/**
 * Dispatch between the type and a lvalue reference to a constant.
 *
 * Used for template fuctions where the input argument could be better taken by value (e.g. an int
 * or other arithmetic type), or const reference (e.g. a std::string).
 */
template <typename T>
using value_or_const_ref_t = std::conditional_t<std::is_trivially_copyable<T>::value, T, T const&>;

}  // namespace utility
}  // namespace ecole
