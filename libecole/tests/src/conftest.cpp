#include <stdexcept>

#include <scip/scip.h>

#include "conftest.hpp"

ecole::scip::Model get_model(SCIP_STAGE stage) {
	auto model = ecole::scip::Model::from_file(problem_file);
	model.disable_cuts();
	model.disable_presolve();
	advance_to_stage(model, stage);
	return model;
}

void advance_to_stage(ecole::scip::Model& model, SCIP_STAGE stage) {
	switch (stage) {
	case SCIP_STAGE_PROBLEM:
		break;
	case SCIP_STAGE_TRANSFORMED:
		model.transform_prob();
		break;
	case SCIP_STAGE_PRESOLVED:
		model.presolve();
		break;
	case SCIP_STAGE_SOLVING:
		model.solve_iter();
		break;
	case SCIP_STAGE_SOLVED:
		model.solve();
		break;
	default:
		throw std::logic_error{"Function get_model is not implemented for given stage stage "};
	}
}
