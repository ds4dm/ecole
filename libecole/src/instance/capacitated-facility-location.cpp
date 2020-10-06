#include "ecole/instance/capacitated-facility-location.hpp"

namespace ecole::instance {

scip::Model CapacitatedFacilityLocationGenerator::generate_instance(
	RandomEngine& /*random_engine*/,
	CapacitatedFacilityLocationGenerator::Parameters /*parameters*/) {
	// TODO implement algorithm
	return {};
}

CapacitatedFacilityLocationGenerator::CapacitatedFacilityLocationGenerator(
	RandomEngine random_engine_,
	CapacitatedFacilityLocationGenerator::Parameters parameters_) :
	random_engine{random_engine_}, parameters{parameters_} {}
CapacitatedFacilityLocationGenerator::CapacitatedFacilityLocationGenerator(
	CapacitatedFacilityLocationGenerator::Parameters parameters_) :
	CapacitatedFacilityLocationGenerator{ecole::spawn_random_engine(), parameters_} {}
CapacitatedFacilityLocationGenerator::CapacitatedFacilityLocationGenerator() :
	CapacitatedFacilityLocationGenerator(Parameters{}) {}

scip::Model CapacitatedFacilityLocationGenerator::next() {
	return generate_instance(random_engine, parameters);
}

void CapacitatedFacilityLocationGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

}  // namespace ecole::instance
