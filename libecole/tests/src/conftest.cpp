#include <scip/scip.h>

#include "conftest.hpp"

ScipNoErrorGuard::ScipNoErrorGuard() {
	SCIPmessageSetErrorPrinting(nullptr, nullptr);
}
ScipNoErrorGuard::~ScipNoErrorGuard() {
	SCIPmessageSetErrorPrintingDefault();
}
