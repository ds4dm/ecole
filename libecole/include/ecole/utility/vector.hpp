#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace ecole::utility {

/** Return vector with values 0 to n (excluded). */
template <typename T = std::size_t> auto arange(std::size_t n) -> std::vector<T> {
	auto indices = std::vector<T>(n);
	std::generate(indices.begin(), indices.end(), [i = T{0}]() mutable { return i++; });
	return indices;
}

}  // namespace ecole::utility
