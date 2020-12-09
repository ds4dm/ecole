#pragma once

#include <array>
#include <cstddef>
#include <tuple>

#include <xtensor/xtensor.hpp>

namespace ecole::utility {

/**
 * Simple coordinate sparse matrix.
 *
 * Indices are given with shape (2, nnz, that is indices[0] are row indices and indices[1] are columns indicies.
 *
 * FIXME there is early development of a sparse xtensor to replace this class, but it still a bit early.
 * https://github.com/xtensor-stack/xtensor-sparse
 */
template <typename T> struct coo_matrix {
	using value_type = T;

	xt::xtensor<value_type, 1> values;
	xt::xtensor<std::size_t, 2> indices;
	std::array<std::size_t, 2> shape = {0, 0};

	using Tuple = std::tuple<decltype(values), decltype(indices), decltype(shape)>;

	[[nodiscard]] static auto from_tuple(Tuple t) -> coo_matrix;

	[[nodiscard]] auto to_tuple() const& -> Tuple;
	[[nodiscard]] auto to_tuple() && -> Tuple;

	[[nodiscard]] auto nnz() const noexcept -> std::size_t { return values.size(); }

	auto operator==(coo_matrix const& other) const -> bool;
};

/**********************************
 *  Implementation of coo_matrix  *
 **********************************/

template <typename T> auto coo_matrix<T>::from_tuple(Tuple t) -> coo_matrix {
	return std::apply([](auto&&... vals) { return coo_matrix{std::forward<decltype(vals)>(vals)...}; }, std::move(t));
}

template <typename T> auto coo_matrix<T>::to_tuple() const& -> Tuple {
	return {values, indices, shape};
}
template <typename T> auto coo_matrix<T>::to_tuple() && -> Tuple {
	return {std::move(values), std::move(indices), shape};
}

template <typename T> auto coo_matrix<T>::operator==(coo_matrix const& other) const -> bool {
	return std::tie(values, indices, shape) == std::tie(other.values, other.indices, other.shape);
}

}  // namespace ecole::utility
