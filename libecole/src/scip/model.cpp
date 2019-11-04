#include <cstddef>
#include <exception>
#include <string>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/exception.hpp"
#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"

struct SCIP_BranchruleData {
	ecole::scip::Model::BranchFunc func;
	ecole::scip::Model const& model;
};

namespace ecole {
namespace scip {

template <> void Deleter<Scip>::operator()(Scip* scip) { call(SCIPfree, &scip); }

unique_ptr<Scip> create() {
	Scip* scip_raw;
	call(SCIPcreate, &scip_raw);
	SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip_raw), true);
	auto scip_ptr = unique_ptr<Scip>{};
	scip_ptr.reset(scip_raw);
	return scip_ptr;
}

unique_ptr<Scip> copy(Scip const* source) {
	if (!source)
		return nullptr;
	if (SCIPgetStage(const_cast<Scip*>(source)) == SCIP_STAGE_INIT)
		return create();
	auto dest = create();
	call(
		SCIPcopy,
		const_cast<Scip*>(source),
		dest.get(),
		nullptr,
		nullptr,
		"",
		true,
		false,
		false,
		nullptr);
	return dest;
}

Model::Model() : scip(create()) { call(SCIPincludeDefaultPlugins, scip.get()); }

Model::Model(unique_ptr<Scip>&& scip) {
	if (scip)
		this->scip = std::move(scip);
	else
		throw Exception("Cannot create empty model");
}

Model::Model(Model const& other) : scip(copy(other.scip.get())) {}

Model& Model::operator=(Model const& other) {
	if (&other != this)
		scip = copy(other.scip.get());
	return *this;
}

Model Model::from_file(const std::string& filename) {
	auto model = Model{};
	call(SCIPreadProb, model.scip.get(), filename.c_str(), nullptr);
	return model;
}

template <> void Model::set_param(const char* name, bool value) {
	call(SCIPsetBoolParam, scip.get(), name, value);
}
template <> void Model::set_param(const char* name, char value) {
	call(SCIPsetCharParam, scip.get(), name, value);
}
template <> void Model::set_param(const char* name, int value) {
	call(SCIPsetIntParam, scip.get(), name, value);
}
template <> void Model::set_param(const char* name, SCIP_Longint value) {
	call(SCIPsetLongintParam, scip.get(), name, value);
}
template <> void Model::set_param(const char* name, SCIP_Real value) {
	call(SCIPsetRealParam, scip.get(), name, value);
}
template <> void Model::set_param(const char* name, const char* value) {
	call(SCIPsetStringParam, scip.get(), name, value);
}

namespace {
template <typename T> SCIP_RETCODE _get_param(SCIP* scip, const char* name, T* value);

template <> SCIP_RETCODE _get_param(SCIP* scip, const char* name, char* value) {
	return SCIPgetCharParam(scip, name, value);
}
template <> SCIP_RETCODE _get_param(SCIP* scip, const char* name, int* value) {
	return SCIPgetIntParam(scip, name, value);
}
template <> SCIP_RETCODE _get_param(SCIP* scip, const char* name, SCIP_Longint* value) {
	return SCIPgetLongintParam(scip, name, value);
}
template <> SCIP_RETCODE _get_param(SCIP* scip, const char* name, SCIP_Real* value) {
	return SCIPgetRealParam(scip, name, value);
}
} // namespace

template <typename T> T Model::get_param(const char* name) {
	T value{};
	call(_get_param<T>, scip.get(), name, &value);
	return value;
}
template char Model::get_param(const char* name);
template int Model::get_param(const char* name);
template SCIP_Longint Model::get_param(const char* name);
template SCIP_Real Model::get_param(const char* name);

template <> bool Model::get_param(const char* name) {
	SCIP_Bool value{};
	call(SCIPgetBoolParam, scip.get(), name, &value);
	return value;
}

template <> std::string Model::get_param(const char* name) {
	char* ptr = nullptr;
	call(SCIPgetStringParam, scip.get(), name, &ptr);
	return ptr;
}

void Model::solve() { call(SCIPsolve, scip.get()); }

void Model::disable_presolve() {
	call(SCIPsetPresolving, scip.get(), SCIP_PARAMSETTING_OFF, true);
}
void Model::disable_cuts() {
	call(SCIPsetSeparating, scip.get(), SCIP_PARAMSETTING_OFF, true);
}

bool Model::is_solved() const noexcept {
	return SCIPgetStage(scip.get()) == SCIP_STAGE_SOLVED;
}

VarView Model::variables() const noexcept {
	auto n_vars = static_cast<std::size_t>(SCIPgetNVars(scip.get()));
	return VarView(SCIPgetVars(scip.get()), n_vars);
}

VarView Model::lp_branch_vars() const noexcept {
	int n_vars{};
	SCIP_VAR** vars{};
	call(
		SCIPgetLPBranchCands, scip.get(), &vars, nullptr, nullptr, &n_vars, nullptr, nullptr);
	return VarView(vars, static_cast<std::size_t>(n_vars));
}

class Model::LambdaBranchRule {
private:
	static constexpr auto name = "ecole::scip::LambdaBranchRule";
	static constexpr auto description = "";
	static constexpr auto priority = 536870911; // Maximum branching rule priority
	static constexpr auto maxdepth = -1;        // No maximum depth
	static constexpr auto maxbounddist = 1.0;   // No distance to dual bound

	static auto exec_lp(
		SCIP* scip,
		SCIP_BRANCHRULE* branch_rule,
		SCIP_Bool allow_addcons,
		SCIP_RESULT* result) {
		(void)allow_addcons;
		auto const branch_data = SCIPbranchruleGetData(branch_rule);
		assert(branch_data->model.scip.get() == scip);
		*result = SCIP_DIDNOTRUN;
		if (!branch_data->func)
			return SCIP_OKAY;

		// C code must be exception safe.
		try {
			auto var = branch_data->func(branch_data->model);
			call(SCIPbranchVar, scip, var.value, nullptr, nullptr, nullptr);
			*result = SCIP_BRANCHED;
		} catch (std::exception& e) {
			SCIPerrorMessage(e.what());
			return SCIP_BRANCHERROR;
		} catch (...) {
			return SCIP_BRANCHERROR;
		}
		return SCIP_OKAY;
	}

	static auto include_void_branch_rule(Model& model) {
		auto const scip = model.scip.get();
		SCIP_BRANCHRULE* branch_rule;
		call(
			SCIPincludeBranchruleBasic,
			scip,
			&branch_rule,
			name,
			description,
			priority,
			maxdepth,
			maxbounddist,
			new SCIP_BranchruleData{Model::BranchFunc{nullptr}, model});
		call(SCIPsetBranchruleExecLp, scip, branch_rule, exec_lp);
		return branch_rule;
	}

	static inline auto get_branch_rule(Model const& model) {
		return SCIPfindBranchrule(model.scip.get(), name);
	}

	static void
	set_branch_func(SCIP_BRANCHRULE* const branch_rule, Model::BranchFunc const& func) {
		auto const branch_data = SCIPbranchruleGetData(branch_rule);
		branch_data->func = func;
	}

public:
	LambdaBranchRule() = delete;

	static void set_branch_func(Model& model, Model::BranchFunc const& func) {
		auto branch_rule = get_branch_rule(model);
		if (!branch_rule)
			branch_rule = include_void_branch_rule(model);
		set_branch_func(branch_rule, func);
	}
};

char const* const Model::LambdaBranchRule::name;
char const* const Model::LambdaBranchRule::description;
int const Model::LambdaBranchRule::priority;
int const Model::LambdaBranchRule::maxdepth;
double const Model::LambdaBranchRule::maxbounddist;

void Model::set_branch_rule(BranchFunc const& func) {
	LambdaBranchRule::set_branch_func(*this, func);
}

} // namespace scip
} // namespace ecole
