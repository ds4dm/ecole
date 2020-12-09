#include <catch2/catch.hpp>

#include "ecole/utility/sparse-matrix.hpp"

using namespace ecole;

TEST_CASE("Sparse matrix unit unit tests", "[unit][utility]") {
	auto const matrix = utility::coo_matrix<double>{
		{2., 4., 7.},            // NOLINT(readability-magic-numbers)
		{{0, 1, 1}, {1, 1, 2}},  // NOLINT(readability-magic-numbers)
		{2, 2},                  // NOLINT(readability-magic-numbers)
	};

	SECTION("Equality comparison") {
		auto const matrix_copy = matrix;  // NOLINT(performance-unnecessary-copy-initialization)
		REQUIRE(matrix_copy == matrix);
	}

	SECTION("To and from tuple") {
		auto t = matrix.to_tuple();
		REQUIRE((std::get<decltype(matrix.indices)>(t) == matrix.indices));
		REQUIRE((std::get<decltype(matrix.values)>(t) == matrix.values));
		REQUIRE((std::get<decltype(matrix.shape)>(t) == matrix.shape));
		auto const matrix_copy = utility::coo_matrix<double>::from_tuple(std::move(t));
		REQUIRE(matrix_copy == matrix);
	}
}
