#include <cstddef>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/exception.hpp"
#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"

struct SCIP_BranchruleData {
	ecole::scip::BranchFunc func;
};

namespace ecole {
namespace scip {

template <> void Deleter<Scip>::operator()(Scip* scip) { call(SCIPfree, &scip); }

unique_ptr<Scip> create() {
	Scip* scip_raw;
	call(SCIPcreate, &scip_raw);
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

Model::Model(unique_ptr<Scip>&& scip) {
	if (scip)
		this->scip = std::move(scip);
	else
		throw ScipException("Cannot create empty model");
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

void Model::solve() { call(SCIPsolve, scip.get()); }

void Model::disable_presolve() {
	call(SCIPsetPresolving, scip.get(), SCIP_PARAMSETTING_OFF, true);
}
void Model::disable_cuts() {
	call(SCIPsetSeparating, scip.get(), SCIP_PARAMSETTING_OFF, true);
}

std::size_t Model::n_vars() const noexcept {
	return static_cast<std::size_t>(SCIPgetNVars(scip.get()));
}

VarView Model::variables() const noexcept {
	return VarView(SCIPgetVars(scip.get()), n_vars());
}

class LambdaBranchRule {
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
		(void)scip;
		(void)allow_addcons;
		(void)result;
		auto const branch_data = SCIPbranchruleGetData(branch_rule);
		// FIXME add try catch
		branch_data->func();
		return SCIP_OKAY;
	}

	static auto include_void_branch_rule(SCIP* const scip) {
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
			new SCIP_BranchruleData{BranchFunc{nullptr}});
		call(SCIPsetBranchruleExecLp, scip, branch_rule, exec_lp);
		return branch_rule;
	}

	static inline auto get_branch_rule(SCIP const* const scip) {
		return SCIPfindBranchrule(const_cast<SCIP*>(scip), name);
	}

	static void
	set_branch_func(SCIP_BRANCHRULE* const branch_rule, BranchFunc const& func) {
		auto const branch_data = SCIPbranchruleGetData(branch_rule);
		branch_data->func = func;
	}

public:
	LambdaBranchRule() = delete;

	static void set_branch_func(unique_ptr<Scip> const& scip, BranchFunc const& func) {
		auto branch_rule = get_branch_rule(scip.get());
		if (!branch_rule)
			branch_rule = include_void_branch_rule(scip.get());
		set_branch_func(branch_rule, func);
	}
};

char const* const LambdaBranchRule::name;
char const* const LambdaBranchRule::description;
int const LambdaBranchRule::priority;
int const LambdaBranchRule::maxdepth;
double const LambdaBranchRule::maxbounddist;

void Model::set_branch_rule(BranchFunc const& func) {
	LambdaBranchRule::set_branch_func(scip, func);
}

} // namespace scip
} // namespace ecole
