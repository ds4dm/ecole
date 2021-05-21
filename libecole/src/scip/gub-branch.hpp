#pragma once

#include <scip/scip.h>

SCIP_RETCODE
SCIPbranchGUB(SCIP* scip, SCIP_VAR** vars, int nvars, SCIP_NODE** downchild, SCIP_NODE** upchild);
