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

/**
 * Return a Model that is in stage solving.
 */
ecole::scip::Model get_solving_model();
