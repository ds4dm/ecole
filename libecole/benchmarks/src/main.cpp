#include <iostream>
#include <iterator>
#include <type_traits>
#include <vector>

#include <nlohmann/json.hpp>

#include "ecole/instance/capacitated-facility-location.hpp"
#include "ecole/instance/combinatorial-auction.hpp"
#include "ecole/instance/independent-set.hpp"
#include "ecole/instance/set-cover.hpp"

#include "bench-branching.hpp"
#include "benchmark.hpp"

using namespace ecole::benchmark;
using namespace ecole::instance;

template <typename... T> auto concat(std::vector<T>... vecs) {
	using U = std::common_type_t<T...>;
	auto result = std::vector<U>{};
	result.reserve((vecs.size() + ...));
	auto inserter = std::back_inserter(result);
	((std::move(vecs.begin(), vecs.end(), inserter)), ...);
	return result;
}

template <typename Generator> auto make_generator(Generator generator) {
	auto result = [generator = std::move(generator)]() mutable {
		auto model = generator.next();
		model.set_param("limits/totalnodes", 100);
		return model;
	};
	return result;
}

auto benchmark_branching(std::size_t n) {
	using GraphType = typename ecole::instance::IndependentSetGenerator::Parameters::GraphType;
	return concat(
		// clang-format off
		benchmark_branching(make_generator(SetCoverGenerator{{500, 1000}}), n, {"SetCover", "Easy"}),
		benchmark_branching(make_generator(SetCoverGenerator{{1000, 1000}}), n, {"SetCover", "Medium"}),
		benchmark_branching(make_generator(SetCoverGenerator{{2000, 1000}}), n, {"SetCover", "Hard"}),
		benchmark_branching(make_generator(CombinatorialAuctionGenerator{{100, 500}}), n, {"CombinatorialAuction", "Easy"}),
		benchmark_branching(make_generator(CombinatorialAuctionGenerator{{200, 1000}}), n, {"CombinatorialAuction", "Medium"}),
		benchmark_branching(make_generator(CombinatorialAuctionGenerator{{300, 1500}}), n, {"CombinatorialAuction", "Hard"}),
		benchmark_branching(make_generator(CapacitatedFacilityLocationGenerator{{100, 100}}), n, {"CapacitatedFacilityLocation", "Easy"}),
		benchmark_branching(make_generator(CapacitatedFacilityLocationGenerator{{200, 100}}), n, {"CapacitatedFacilityLocation", "Medium"}),
		benchmark_branching(make_generator(CapacitatedFacilityLocationGenerator{{400, 100}}), n, {"CapacitatedFacilityLocation", "Hard"}),
		benchmark_branching(make_generator(IndependentSetGenerator{{500, GraphType::erdos_renyi}}), n, {"IndependentSet", "Easy"}),
		benchmark_branching(make_generator(IndependentSetGenerator{{1000, GraphType::erdos_renyi}}), n, {"IndependentSet", "Medium"}),
		benchmark_branching(make_generator(IndependentSetGenerator{{1500, GraphType::erdos_renyi}}), n, {"IndependentSet", "Hard"})
		// clang-format on
	);
}

int main() {
	auto vec = benchmark_branching(1);
	auto json = nlohmann::json{};
	json = vec;
	std::cout << json << '\n';
}
