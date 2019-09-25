#include <cstddef>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"

namespace ecole {
namespace scip {

void ScipDeleter::operator()(Scip* scip) { call(SCIPfree, &scip); }

std::unique_ptr<Scip, ScipDeleter> create() {
	Scip* scip_raw;
	call(SCIPcreate, &scip_raw);
	auto scip_ptr = std::unique_ptr<Scip, ScipDeleter>{};
	scip_ptr.reset(scip_raw);
	return scip_ptr;
}

Model::Model() : scip(create()) {
	SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip.get()), TRUE);
	call(SCIPincludeDefaultPlugins, scip.get());
}

Model Model::from_file(const std::string& filename) {
	auto model = Model{};
	call(SCIPreadProb, model.scip.get(), filename.c_str(), nullptr);
	return model;
}

void Model::solve() { call(SCIPsolve, scip.get()); }

std::size_t Model::n_vars() const noexcept {
	return static_cast<std::size_t>(SCIPgetNVars(scip.get()));
}

VarView Model::variables() const noexcept {
	return VarView(SCIPgetVars(scip.get()), n_vars());
}

} // namespace scip
} // namespace ecole
