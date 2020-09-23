#include "ecole/instance/set-cover.hpp"

namespace ecole::instance {

scip::Model SetCoverGenerator::generate_instance(Parameters /*parameters*/, RandomEngine& /*random_engine*/) {
	// TODO implement algorithm
	return {};
}

SetCoverGenerator::SetCoverGenerator(Parameters parameters_, RandomEngine random_engine_) :
	random_engine{random_engine_}, parameters{parameters_} {}
SetCoverGenerator::SetCoverGenerator(Parameters parameters) :
	SetCoverGenerator{parameters, RandomEngine{std::random_device{}()}} {}
SetCoverGenerator::SetCoverGenerator() : SetCoverGenerator(Parameters{}) {}

scip::Model SetCoverGenerator::next() {
	return generate_instance(parameters, random_engine);
}

}  // namespace ecole::instance
