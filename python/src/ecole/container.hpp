#pragma once

#include <cstddef>

#include <xtensor-python/pytensor.hpp>

#include <ecole/container.hpp>

namespace ecole {
namespace container {

struct pytensor {};

template <typename T, std::size_t N> struct tensor_container<pytensor, T, N> {
	using type = xt::pytensor<T, N, xt::layout_type::row_major>;
};

}  // namespace container
}  // namespace ecole
