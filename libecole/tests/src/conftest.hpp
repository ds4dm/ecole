#pragma once

#include <string>

#include <scip/scip.h>

#include "ecole/scip/model.hpp"

#ifndef TEST_DATA_DIR
#error "Need to define TEST_DATA_DIR."
#endif
constexpr auto problem_file = (TEST_DATA_DIR "/bppc8-02.mps");

/**
 * Return a Model that is not trivially solved in the deisred stage.
 */
ecole::scip::Model get_model(SCIP_STAGE stage = SCIP_STAGE_PROBLEM);

/*
 * Advance an unsolved Model to the root node.
 */
void advance_to_stage(ecole::scip::Model& model, SCIP_STAGE stage);
