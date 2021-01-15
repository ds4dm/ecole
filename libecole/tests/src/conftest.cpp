#include <scip/scip.h>

#include "conftest.hpp"

ecole::scip::Model get_model() {
	auto model = ecole::scip::Model::from_file(problem_file);
	model.disable_cuts();
	model.disable_presolve();
	return model;
}

void advance_to_root_node(ecole::scip::Model& model) {
	model.solve_iter();
}
