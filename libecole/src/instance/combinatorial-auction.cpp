#include "ecole/instance/combinatorial-auction.hpp"

namespace ecole::instance {

scip::Model
CombinatorialAuctionGenerator::generate_instance(RandomEngine& /*random_engine*/, Parameters /*parameters*/) {
	// TODO implement algorithm
	return {};
}

CombinatorialAuctionGenerator::CombinatorialAuctionGenerator(RandomEngine random_engine_, Parameters parameters_) :
	random_engine{random_engine_}, parameters{parameters_} {}
CombinatorialAuctionGenerator::CombinatorialAuctionGenerator(Parameters parameters_) :
	CombinatorialAuctionGenerator{ecole::spawn_random_engine(), parameters_} {}
CombinatorialAuctionGenerator::CombinatorialAuctionGenerator() : CombinatorialAuctionGenerator(Parameters{}) {}

scip::Model CombinatorialAuctionGenerator::next() {
	return generate_instance(random_engine, parameters);
}

void CombinatorialAuctionGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

}  // namespace ecole::instance
