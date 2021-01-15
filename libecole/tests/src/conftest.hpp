#pragma once

#include <string>

#include "ecole/scip/model.hpp"

#ifndef TEST_DATA_DIR
#error "Need to define TEST_DATA_DIR."
#endif
constexpr auto problem_file = (TEST_DATA_DIR "/bppc8-02.mps");

/**
 * Return a Model that is not trivially solved.
 */
ecole::scip::Model get_model();

/*
 * Advance an unsolved Model to the root node.
 */
void advance_to_root_node(ecole::scip::Model& model);
