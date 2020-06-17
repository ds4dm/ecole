#pragma once

#include <array>
#include <cstddef>

#include <xtensor/xtensor.hpp>

namespace ecole {
namespace utility {

template <typename T> struct coo_matrix {
	using value_type = T;

	xt::xtensor<value_type, 1> values;
	xt::xtensor<std::size_t, 2> indices;
	std::array<std::size_t, 2> shape;

	std::size_t nnz() const noexcept { return values.size(); }
};

}  // namespace utility
}  // namespace ecole
