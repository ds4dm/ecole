#pragma once

#include <algorithm>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "ecole/utility/vector.hpp"

namespace ecole::utility {

/**
 * Sample without replacement according to the given probabilities.
 *
 * FIXME Merged into Xtensor 0.22.0 xt::choice
 *
 * Sample items according to the probability distribution given by normalizing the weights of the items left.
 * Items are not replaced when sampled.
 *
 * Algorithm from
 * Efraimidis PS, Spirakis PG (2006). "Weighted random sampling with a reservoir."
 * Information Processing  Letters, 97 (5), 181-185. ISSN 0020-0190.
 * doi:10.1016/j.ipl.2005.11.003.
 * http://www.sciencedirect.com/science/article/pii/S002001900500298X
 *
 * The keys computed are replaced with weight/randexp(1) instead rand()^(1/weight) as done in wrlmlR:
 * https://web.archive.org/web/20201021162211/https://krlmlr.github.io/wrswoR/
 * https://web.archive.org/web/20201021162520/https://github.com/krlmlr/wrswoR/blob/master/src/sample_int_crank.cpp
 * As well as in JuliaStats:
 * https://web.archive.org/web/20201021162949/https://github.com/JuliaStats/StatsBase.jl/blob/master/src/sampling.jl
 *
 * @tparam T Type of the weights is used to make computation.
 * @param n_samples Number of items to sample without replacement.
 * @param weights The weights of each items (implicty their index).
 * @param random_engine The source of randomness used to sample.
 * @return A vector of the n_samples items selected as their index in the weights vector.
 */
template <typename T, typename RandomEngine>
auto arg_choice(std::size_t n_samples, std::vector<T> weights, RandomEngine& random_engine) {
	static_assert(std::is_floating_point_v<T>, "Weights must be real numbers.");

	auto const n_items = weights.size();
	if (n_samples > n_items) {
		throw std::invalid_argument{"Cannot sample more than there are items."};
	}

	// Compute (modified) keys as weight/randexp(1) reusing weights vector.
	auto randexp = std::exponential_distribution<T>{1.};
	for (auto& w : weights) {
		w /= randexp(random_engine);
	}

	// Sort an array of indices using -keys[i] as comparing value.
	// We only the top n_sample largest keys.
	auto indices = arange(n_items);
	auto compare = [&weights](auto i, auto j) { return weights[i] > weights[j]; };
	std::partial_sort(indices.begin(), indices.begin() + static_cast<std::ptrdiff_t>(n_items), indices.end(), compare);
	indices.resize(n_samples);

	return indices;
}

}  // namespace ecole::utility
