#pragma once

#include <utility>

#include <nonstd/span.hpp>
#include <range/v3/range_fwd.hpp>

/**
 * Tell the range library that `nonstd::span` is a view type.
 *
 * See `Rvalue Ranges and Views in C++20 <https://tristanbrindle.com/posts/rvalue-ranges-and-views>`_
 * FIXME no longer needed when switching to C++20 ``std::span``.
 * */
namespace ranges {
template <typename T, std::size_t Extent> inline constexpr bool enable_borrowed_range<nonstd::span<T, Extent>> = true;
}  // namespace ranges
