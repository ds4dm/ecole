#pragma once

#include <array>
#include <cstddef>

#include <xtensor/xtensor.hpp>

namespace ecole::utility {

template <typename T> struct coo_matrix {
	using value_type = T;

	xt::xtensor<value_type, 1> values;
	xt::xtensor<std::size_t, 2> indices;
	std::array<std::size_t, 2> shape;

	[[nodiscard]] std::size_t nnz() const noexcept { return values.size(); }
};

}  // namespace ecole::utility
