#include <cstddef>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/exception.hpp"
#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"

namespace ecole {
namespace scip {

void ScipDeleter::operator()(Scip* scip) { call(SCIPfree, &scip); }

using ScipPtr = std::unique_ptr<Scip, ScipDeleter>;

ScipPtr create() {
	Scip* scip_raw;
	call(SCIPcreate, &scip_raw);
	auto scip_ptr = ScipPtr{};
	scip_ptr.reset(scip_raw);
	return scip_ptr;
}

ScipPtr copy(ScipPtr const& source) {
	if (!source)
		return nullptr;
	if (SCIPgetStage(source.get()) == SCIP_STAGE_INIT)
		return create();
	auto dest = create();
	call(
		SCIPcopy,
		source.get(),
		dest.get(),
		nullptr,
		nullptr,
		nullptr,
		true,
		false,
		false,
		nullptr);
	return dest;
}

Model::Model() : scip(create()) {
	SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip.get()), TRUE);
	call(SCIPincludeDefaultPlugins, scip.get());
}

Model::Model(ScipPtr&& scip) {
	if (scip)
		this->scip = std::move(scip);
	else
		throw ScipException("Cannot create empty model");
}

Model::Model(Model const& other) : scip(copy(other.scip)) {}

Model& Model::operator=(Model const& other) {
	if (&other != this)
		scip = copy(other.scip);
	return *this;
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
