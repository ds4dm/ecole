#pragma once

#include <cstddef>

#include <xtensor/xtensor.hpp>

namespace ecole {
namespace container {

struct xtensor {};

template <typename ContainerSelector, typename T, std::size_t N> struct tensor_container;

template <typename ContainerSelector, typename T, std::size_t N>
using tensor_t = typename tensor_container<ContainerSelector, T, N>::type;

template <typename T, std::size_t N> struct tensor_container<xtensor, T, N> {
	using type = xt::xtensor<T, N, xt::layout_type::row_major>;
};

}  // namespace container
}  // namespace ecole
