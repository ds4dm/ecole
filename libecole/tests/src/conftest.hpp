#pragma once

#include <string>

#ifndef TEST_DATA_DIR
#error "Need to define TEST_DATA_DIR."
#endif
const auto problem_file = static_cast<std::string>(TEST_DATA_DIR "/enlight8.mps");

struct ScipNoErrorGuard {
	ScipNoErrorGuard();
	~ScipNoErrorGuard();
};
