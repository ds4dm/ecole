#include "ecole/instance/set-cover.hpp"

namespace ecole::instance {

scip::Model SetCoverGenerator::generate_instance(RandomEngine& /*random_engine*/, Parameters /*parameters*/) {
	// TODO implement algorithm
	return {};
}

SetCoverGenerator::SetCoverGenerator(RandomEngine random_engine_, Parameters parameters_) :
	random_engine{random_engine_}, parameters{parameters_} {}
SetCoverGenerator::SetCoverGenerator(Parameters parameters_) :
	SetCoverGenerator{ecole::spawn_random_engine(), parameters_} {}
SetCoverGenerator::SetCoverGenerator() : SetCoverGenerator(Parameters{}) {}

scip::Model SetCoverGenerator::next() {
	return generate_instance(random_engine, parameters);
}

void SetCoverGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

}  // namespace ecole::instance
