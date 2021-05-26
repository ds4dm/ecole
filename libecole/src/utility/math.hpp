#pragma once

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <utility>

namespace ecole::utility {

/**
 * Square of a number.
 */
template <typename T> auto square(T x) noexcept -> T {
	return x * x;
}

/**
 * Floating points division that return 0 when the denominator is 0.
 * Also ensures that it isn't accidentally called with integers which would lead to euclidian
 * division.
 */
template <typename T> auto safe_div(T x, T y) noexcept -> T {
	static_assert(std::is_floating_point_v<T>, "Inputs are not decimals");
	return y != 0. ? x / y : 0.;
}

namespace internal {

template <typename R>
using range_value_type_t = std::remove_cv_t<std::remove_reference_t<decltype(*std::declval<R>().begin())>>;

}  // namespace internal

/**
 * Compute the sum of and count of elements.
 *
 * @param range The container to iteratre over.
 * @param transform A callable to apply on every element of the container.
 * @param filter A callable to filter in elements (after transformation).
 */
template <typename Range, typename U = internal::range_value_type_t<Range>>
auto count_sum(Range range) noexcept -> std::pair<U, std::size_t> {
	auto sum = U{0};
	auto count = std::size_t{0};

	for (auto const element : range) {
		count++;
		sum += element;
	}
	return {count, sum};
}

/**
 * Hold statistics of a range.
 */
template <typename T> struct StatsFeatures {
	T count = 0.;
	T sum = 0.;
	T mean = 0.;
	T stddev = 0.;
	T min = 0.;
	T max = 0.;
};

template <
	typename Range,
	typename U = internal::range_value_type_t<Range>,
	typename T = std::conditional_t<std::is_floating_point_v<U>, U, double>>
auto compute_stats(Range range) noexcept -> StatsFeatures<T> {
	auto const [count, sum] = count_sum(range);

	// We can assume count to be always positive after this point and that the (filtered) iteration
	// will contain at least one element.
	if (count == 0) {
		return {};
	}

	auto const mean = safe_div(static_cast<T>(sum), static_cast<T>(count));
	auto stddev = T{0.};
	auto min = std::numeric_limits<U>::max();
	auto max = std::numeric_limits<U>::min();

	for (auto const element : range) {
		min = std::min(min, element);
		max = std::max(max, element);
		stddev += square(static_cast<T>(element) - mean);
	}
	stddev = std::sqrt(stddev / static_cast<T>(count));

	return {static_cast<T>(count), static_cast<T>(sum), mean, stddev, static_cast<T>(min), static_cast<T>(max)};
}

}  // namespace ecole::utility
