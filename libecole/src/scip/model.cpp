#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iterator>
#include <string>

#include <fmt/format.h>
#include <range/v3/view/move.hpp>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/exception.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/scimpl.hpp"

#include "ecole/scip/utils.hpp"

namespace ecole::scip {

Model::Model() : scimpl(std::make_unique<Scimpl>()) {}

Model::Model(Model&&) noexcept = default;

Model::Model(std::unique_ptr<Scimpl>&& other_scimpl) : scimpl(std::move(other_scimpl)) {}

Model::~Model() = default;

Model& Model::operator=(Model&&) noexcept = default;

SCIP* Model::get_scip_ptr() noexcept {
	return scimpl->get_scip_ptr();
}
SCIP const* Model::get_scip_ptr() const noexcept {
	return scimpl->get_scip_ptr();
}

Model Model::copy_orig() const {
	return std::make_unique<Scimpl>(scimpl->copy_orig());
}

bool Model::operator==(Model const& other) const noexcept {
	return scimpl == other.scimpl;
}

bool Model::operator!=(Model const& other) const noexcept {
	return !(*this == other);
}

Model Model::from_file(const std::string& filename) {
	auto model = Model{};
	model.read_problem(filename);
	return model;
}

Model Model::prob_basic(std::string const& name) {
	auto model = Model{};
	scip::call(SCIPcreateProbBasic, model.get_scip_ptr(), name.c_str());
	return model;
}

void Model::write_problem(const std::string& filename) const {
	scip::call(SCIPwriteOrigProblem, const_cast<SCIP*>(get_scip_ptr()), filename.c_str(), nullptr, true);
}

void Model::read_problem(std::string const& filename) {
	scip::call(SCIPreadProb, get_scip_ptr(), filename.c_str(), nullptr);
}

std::string Model::name() const noexcept {
	return SCIPgetProbName(const_cast<SCIP*>(get_scip_ptr()));
}

void Model::set_name(std::string const& name) {
	scip::call(SCIPsetProbName, get_scip_ptr(), name.c_str());
}

SCIP_STAGE Model::get_stage() const noexcept {
	return SCIPgetStage(const_cast<SCIP*>(get_scip_ptr()));
}

ParamType Model::get_param_type(std::string const& name) const {
	auto* scip_param = SCIPgetParam(const_cast<SCIP*>(get_scip_ptr()), name.c_str());
	if (scip_param == nullptr) {
		throw scip::Exception(fmt::format("parameter <{}> unknown", name));
	}
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
		throw Exception(fmt::format("Could not find type for parameter '{}'", name));
	}
}

template <> void Model::set_param<ParamType::Bool>(std::string const& name, bool value) {
	scip::call(SCIPsetBoolParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::Int>(std::string const& name, int value) {
	scip::call(SCIPsetIntParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::LongInt>(std::string const& name, SCIP_Longint value) {
	scip::call(SCIPsetLongintParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::Real>(std::string const& name, SCIP_Real value) {
	scip::call(SCIPsetRealParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::Char>(std::string const& name, char value) {
	scip::call(SCIPsetCharParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::String>(std::string const& name, std::string const& value) {
	scip::call(SCIPsetStringParam, get_scip_ptr(), name.c_str(), value.c_str());
}

template <> bool Model::get_param<ParamType::Bool>(std::string const& name) const {
	SCIP_Bool value{};
	scip::call(SCIPgetBoolParam, const_cast<SCIP*>(get_scip_ptr()), name.c_str(), &value);
	return static_cast<bool>(value);
}
template <> int Model::get_param<ParamType::Int>(std::string const& name) const {
	int value{};
	scip::call(SCIPgetIntParam, const_cast<SCIP*>(get_scip_ptr()), name.c_str(), &value);
	return value;
}
template <> SCIP_Longint Model::get_param<ParamType::LongInt>(std::string const& name) const {
	SCIP_Longint value{};
	scip::call(SCIPgetLongintParam, const_cast<SCIP*>(get_scip_ptr()), name.c_str(), &value);
	return value;
}
template <> SCIP_Real Model::get_param<ParamType::Real>(std::string const& name) const {
	SCIP_Real value{};
	scip::call(SCIPgetRealParam, const_cast<SCIP*>(get_scip_ptr()), name.c_str(), &value);
	return value;
}
template <> char Model::get_param<ParamType::Char>(std::string const& name) const {
	char value{};
	scip::call(SCIPgetCharParam, const_cast<SCIP*>(get_scip_ptr()), name.c_str(), &value);
	return value;
}
template <> std::string Model::get_param<ParamType::String>(std::string const& name) const {
	char* ptr{};
	scip::call(SCIPgetStringParam, const_cast<SCIP*>(get_scip_ptr()), name.c_str(), &ptr);
	return ptr;
}

void Model::set_params(std::map<std::string, Param> name_values) {
	for (auto&& [name, value] : ranges::views::move(name_values)) {
		set_param(name, std::move(value));
	}
}

namespace {

nonstd::span<SCIP_PARAM*> get_params_span(Model const& model) noexcept {
	auto* const scip = const_cast<SCIP*>(model.get_scip_ptr());
	return {SCIPgetParams(scip), static_cast<std::size_t>(SCIPgetNParams(scip))};
}

}  // namespace

std::map<std::string, Param> Model::get_params() const {
	std::map<std::string, Param> name_values{};
	for (auto* const param : get_params_span(*this)) {
		auto name = std::string{SCIPparamGetName(param)};
		auto value = get_param<Param>(name);
		name_values.insert({std::move(name), std::move(value)});
	}
	return name_values;
}

void Model::transform_prob() {
	scip::call(SCIPtransformProb, get_scip_ptr());
}

void Model::presolve() {
	scip::call(SCIPpresolve, get_scip_ptr());
}

void Model::solve() {
	scip::call(SCIPsolve, get_scip_ptr());
}

bool Model::is_solved() const noexcept {
	return SCIPgetStage(const_cast<SCIP*>(get_scip_ptr())) == SCIP_STAGE_SOLVED;
}

void Model::solve_iter() {
	scimpl->solve_iter();
}

void Model::solve_iter_branch(SCIP_VAR* var) {
	scimpl->solve_iter_branch(var);
}

void Model::solve_iter_stop() {
	scimpl->solve_iter_stop();
}

bool Model::solve_iter_is_done() {
	return scimpl->solve_iter_is_done();
}

void Model::disable_presolve() {
	scip::call(SCIPsetPresolving, get_scip_ptr(), SCIP_PARAMSETTING_OFF, true);
}
void Model::disable_cuts() {
	scip::call(SCIPsetSeparating, get_scip_ptr(), SCIP_PARAMSETTING_OFF, true);
}

nonstd::span<SCIP_VAR*> Model::variables() const noexcept {
	auto* const scip_ptr = const_cast<SCIP*>(get_scip_ptr());
	return {SCIPgetVars(scip_ptr), static_cast<std::size_t>(SCIPgetNVars(scip_ptr))};
}

nonstd::span<SCIP_VAR*> Model::lp_branch_cands() const {
	int n_vars = 0;
	SCIP_VAR** vars = nullptr;
	scip::call(
		SCIPgetLPBranchCands, const_cast<SCIP*>(get_scip_ptr()), &vars, nullptr, nullptr, &n_vars, nullptr, nullptr);
	return {vars, static_cast<std::size_t>(n_vars)};
}

nonstd::span<SCIP_VAR*> Model::pseudo_branch_cands() const {
	int n_vars = 0;
	SCIP_VAR** vars = nullptr;
	scip::call(SCIPgetPseudoBranchCands, const_cast<SCIP*>(get_scip_ptr()), &vars, &n_vars, nullptr);
	return {vars, static_cast<std::size_t>(n_vars)};
}

nonstd::span<SCIP_COL*> Model::lp_columns() const {
	auto* const scip_ptr = const_cast<SCIP*>(get_scip_ptr());
	if (SCIPgetStage(scip_ptr) != SCIP_STAGE_SOLVING) {
		throw Exception("LP columns are only available during solving");
	}
	return {SCIPgetLPCols(scip_ptr), static_cast<std::size_t>(SCIPgetNLPCols(scip_ptr))};
}

nonstd::span<SCIP_CONS*> Model::constraints() const noexcept {
	auto* const scip_ptr = const_cast<SCIP*>(get_scip_ptr());
	return {SCIPgetConss(scip_ptr), static_cast<std::size_t>(SCIPgetNConss(scip_ptr))};
}

nonstd::span<SCIP_ROW*> Model::lp_rows() const {
	auto* const scip_ptr = const_cast<SCIP*>(get_scip_ptr());
	if (SCIPgetStage(scip_ptr) != SCIP_STAGE_SOLVING) {
		throw Exception("LP rows are only available during solving");
	}
	return {SCIPgetLPRows(scip_ptr), static_cast<std::size_t>(SCIPgetNLPRows(scip_ptr))};
}

std::size_t Model::nnz() const noexcept {
	return static_cast<std::size_t>(SCIPgetNNZs(const_cast<SCIP*>(get_scip_ptr())));
}

namespace internal {

template <> std::string Caster<std::string, char>::cast(char val) {
	return std::string{val};
}

template <> char Caster<char, char const*>::cast(char const* val) {
	if (strlen(val) == 1) {
		return val[0];
	}
	throw scip::Exception("Can only convert a string with a single character to a char");
}

template <> char Caster<char, std::string>::cast(std::string val) {
	if (val.length() == 1) {
		return val[0];
	}
	throw scip::Exception("Can only convert a string with a single character to a char");
}

}  // namespace internal

}  // namespace ecole::scip
