#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <mutex>
#include <string>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/exception.hpp"
#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"

namespace ecole {
namespace scip {

template <> void Deleter<Scip>::operator()(Scip* scip) {
	scip::call(SCIPfree, &scip);
}

unique_ptr<Scip> create() {
	Scip* scip_raw;
	scip::call(SCIPcreate, &scip_raw);
	SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip_raw), true);
	auto scip_ptr = unique_ptr<Scip>{};
	scip_ptr.reset(scip_raw);
	return scip_ptr;
}

unique_ptr<Scip> copy(Scip const* source) {
	if (!source) return nullptr;
	if (SCIPgetStage(const_cast<Scip*>(source)) == SCIP_STAGE_INIT) return create();
	auto dest = create();
	// Copy operation is not thread safe
	static std::mutex m{};
	std::lock_guard<std::mutex> g{m};
	scip::call(
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

Scip* Model::get_scip_ptr() const noexcept {
	return scip.get();
}

Model::Model() : scip(create()) {
	scip::call(SCIPincludeDefaultPlugins, get_scip_ptr());
}

Model::Model(unique_ptr<Scip>&& scip) {
	if (scip)
		this->scip = std::move(scip);
	else
		throw Exception("Cannot create empty model");
}

Model::Model(Model const& other) : scip(copy(other.get_scip_ptr())) {}

Model& Model::operator=(Model const& other) {
	if (&other != this) scip = copy(other.get_scip_ptr());
	return *this;
}

bool Model::operator==(Model const& other) const noexcept {
	return scip == other.scip;
}

bool Model::operator!=(Model const& other) const noexcept {
	return !(*this == other);
}

Model Model::from_file(const std::string& filename) {
	auto model = Model{};
	scip::call(SCIPreadProb, model.get_scip_ptr(), filename.c_str(), nullptr);
	return model;
}

// Assumptions made while defining ParamType
static_assert(
	std::is_same<SCIP_Bool, param_t<ParamType::Bool>>::value,
	"Scip bool type is not the same as the one redefined by Ecole");
static_assert(
	std::is_same<SCIP_Longint, param_t<ParamType::LongInt>>::value,
	"Scip long int type is not the same as the one redefined by Ecole");
static_assert(
	std::is_same<SCIP_Real, param_t<ParamType::Real>>::value,
	"Scip real type is not the same as the one redefined by Ecole");

ParamType Model::get_param_type(const char* name) const {
	auto* scip_param = SCIPgetParam(get_scip_ptr(), name);
	if (!scip_param)
		throw make_exception(SCIP_PARAMETERUNKNOWN);
	else
		switch (SCIPparamGetType(scip_param)) {
		case SCIP_PARAMTYPE_BOOL:
			return ParamType::Bool;
		case SCIP_PARAMTYPE_INT:
			return ParamType::Int;
		case SCIP_PARAMTYPE_LONGINT:
			return ParamType::LongInt;
		case SCIP_PARAMTYPE_REAL:
			return ParamType::Real;
		case SCIP_PARAMTYPE_CHAR:
			return ParamType::Char;
		case SCIP_PARAMTYPE_STRING:
			return ParamType::String;
		default:
			assert(false);  // All enum value should be handled
			// Non void return for optimized build
			throw Exception("Could not find type for given parameter");
		}
}

ParamType Model::get_param_type(std::string const& name) const {
	return get_param_type(name.c_str());
}

param_t<ParamType::Int> Model::seed() const {
	return get_param_explicit<param_t<ParamType::Int>>("randomization/randomseedshift");
}

template <typename T> static auto mod(T num, T div) noexcept {
	return (num % div + div) % div;
}

void Model::seed(param_t<ParamType::Int> seed_v) {
	using seed_t = param_t<ParamType::Int>;
	set_param_explicit<seed_t>("randomization/randomseedshift", std::abs(seed_v));
}

void Model::solve() {
	scip::call(SCIPsolve, get_scip_ptr());
}

void Model::interrupt_solve() {
	scip::call(SCIPinterruptSolve, get_scip_ptr());
}

void Model::disable_presolve() {
	scip::call(SCIPsetPresolving, get_scip_ptr(), SCIP_PARAMSETTING_OFF, true);
}
void Model::disable_cuts() {
	scip::call(SCIPsetSeparating, get_scip_ptr(), SCIP_PARAMSETTING_OFF, true);
}

bool Model::is_solved() const noexcept {
	return SCIPgetStage(get_scip_ptr()) == SCIP_STAGE_SOLVED;
}

VarView Model::variables() const noexcept {
	auto n_vars = static_cast<std::size_t>(SCIPgetNVars(get_scip_ptr()));
	return VarView(get_scip_ptr(), SCIPgetVars(get_scip_ptr()), n_vars);
}

VarView Model::lp_branch_vars() const noexcept {
	int n_vars{};
	SCIP_VAR** vars{};
	scip::call(
		SCIPgetLPBranchCands,
		get_scip_ptr(),
		&vars,
		nullptr,
		nullptr,
		&n_vars,
		nullptr,
		nullptr);
	return VarView(get_scip_ptr(), vars, static_cast<std::size_t>(n_vars));
}

}  // namespace scip
}  // namespace ecole

struct SCIP_BranchruleData {
	ecole::scip::Model::BranchFunc func;
	ecole::scip::Model& model;
};

namespace ecole {
namespace scip {

class Model::LambdaBranchRule {
	// A Scip branch rule class that runs a given function.
	// The scip BranchRule is actually never substituted, but its internal data is changed
	// to a new function.

private:
	static constexpr auto name = "ecole::scip::LambdaBranchRule";
	static constexpr auto description = "";
	static constexpr auto priority = 536870911;  // Maximum branching rule priority
	static constexpr auto maxdepth = -1;         // No maximum depth
	static constexpr auto maxbounddist = 1.0;    // No distance to dual bound

	static auto exec_lp(
		SCIP* scip,
		SCIP_BRANCHRULE* branch_rule,
		SCIP_Bool allow_addcons,
		SCIP_RESULT* result) {
		// The function that is called to branch on lp fractional varaibles, as required
		// by Scip.
		(void)allow_addcons;
		auto const branch_data = SCIPbranchruleGetData(branch_rule);
		assert(branch_data->model.get_scip_ptr() == scip);
		assert(branch_data->func);
		*result = SCIP_DIDNOTRUN;

		// C code must be exception safe.
		try {
			auto var = branch_data->func(branch_data->model);
			if (var == VarProxy::None)
				*result = SCIP_DIDNOTRUN;
			else {
				scip::call(SCIPbranchVar, scip, var.value, nullptr, nullptr, nullptr);
				*result = SCIP_BRANCHED;
			}
		} catch (std::exception& e) {
			SCIPerrorMessage(e.what());
			return SCIP_BRANCHERROR;
		} catch (...) {
			return SCIP_BRANCHERROR;
		}
		return SCIP_OKAY;
	}

	static auto include_void_branch_rule(Model& model) {
		auto const scip = model.get_scip_ptr();
		SCIP_BRANCHRULE* branch_rule;
		scip::call(
			SCIPincludeBranchruleBasic,
			scip,
			&branch_rule,
			name,
			description,
			priority,
			maxdepth,
			maxbounddist,
			new SCIP_BranchruleData{Model::BranchFunc{nullptr}, model});
		scip::call(SCIPsetBranchruleExecLp, scip, branch_rule, exec_lp);
		return branch_rule;
	}

	static inline auto get_branch_rule(Model const& model) {
		return SCIPfindBranchrule(model.get_scip_ptr(), name);
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
		if (!branch_rule) branch_rule = include_void_branch_rule(model);
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

namespace internal {

template <> void set_scip_param(Scip* scip, const char* name, SCIP_Bool value) {
	scip::call(SCIPsetBoolParam, scip, name, value);
}
template <> void set_scip_param(Scip* scip, const char* name, char value) {
	scip::call(SCIPsetCharParam, scip, name, value);
}
template <> void set_scip_param(Scip* scip, const char* name, int value) {
	scip::call(SCIPsetIntParam, scip, name, value);
}
template <> void set_scip_param(Scip* scip, const char* name, SCIP_Longint value) {
	scip::call(SCIPsetLongintParam, scip, name, value);
}
template <> void set_scip_param(Scip* scip, const char* name, SCIP_Real value) {
	scip::call(SCIPsetRealParam, scip, name, value);
}
template <> void set_scip_param(Scip* scip, const char* name, const char* value) {
	scip::call(SCIPsetStringParam, scip, name, value);
}
template <> void set_scip_param(Scip* scip, const char* name, std::string const& value) {
	return set_scip_param(scip, name, value.c_str());
}

template <> SCIP_Bool get_scip_param(Scip* scip, const char* name) {
	SCIP_Bool value{};
	scip::call(SCIPgetBoolParam, scip, name, &value);
	return value;
}
template <> char get_scip_param(Scip* scip, const char* name) {
	char value{};
	scip::call(SCIPgetCharParam, scip, name, &value);
	return value;
}
template <> int get_scip_param(Scip* scip, const char* name) {
	int value{};
	scip::call(SCIPgetIntParam, scip, name, &value);
	return value;
}
template <> SCIP_Longint get_scip_param(Scip* scip, const char* name) {
	SCIP_Longint value{};
	scip::call(SCIPgetLongintParam, scip, name, &value);
	return value;
}
template <> SCIP_Real get_scip_param(Scip* scip, const char* name) {
	SCIP_Real value{};
	scip::call(SCIPgetRealParam, scip, name, &value);
	return value;
}
template <> const char* get_scip_param(Scip* scip, const char* name) {
	char* ptr = nullptr;
	scip::call(SCIPgetStringParam, scip, name, &ptr);
	return ptr;
}

template <> Cast_SFINAE<char, const char*>::operator char() const {
	if (std::strlen(val) == 1)
		return *val;
	else
		throw scip::Exception("Can only convert a string with a single character to a char");
}

}  // namespace internal

}  // namespace scip
}  // namespace ecole
