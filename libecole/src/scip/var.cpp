#include "ecole/scip/var.hpp"

namespace ecole::scip {

void VarReleaser::operator()(SCIP_VAR* ptr) {
	scip::call(SCIPreleaseVar, scip, &ptr);
}

auto create_var_basic(SCIP* scip, char const* name, SCIP_Real lb, SCIP_Real ub, SCIP_Real obj, SCIP_VARTYPE vartype)
	-> std::unique_ptr<SCIP_VAR, VarReleaser> {
	SCIP_VAR* var = nullptr;
	scip::call(SCIPcreateVarBasic, scip, &var, name, lb, ub, obj, vartype);
	return {var, VarReleaser{scip}};
}

}  // namespace ecole::scip
