#include "ecole/instance/independent-set.hpp"

namespace ecole::instance {

scip::Model IndependentSetGenerator::generate_instance(RandomEngine& /*random_engine*/, Parameters /*parameters*/) {
	// TODO implement algorithm
	return {};
}

IndependentSetGenerator::IndependentSetGenerator(RandomEngine random_engine_, Parameters parameters_) :
	random_engine{random_engine_}, parameters{parameters_} {}
IndependentSetGenerator::IndependentSetGenerator(Parameters parameters_) :
	IndependentSetGenerator{ecole::spawn_random_engine(), parameters_} {}
IndependentSetGenerator::IndependentSetGenerator() : IndependentSetGenerator(Parameters{}) {}

scip::Model IndependentSetGenerator::next() {
	return generate_instance(random_engine, parameters);
}

void IndependentSetGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

}  // namespace ecole::instance
