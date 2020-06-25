#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <string>

#include <fmt/format.h>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/exception.hpp"
#include "ecole/scip/model.hpp"
#include "ecole/scip/scimpl.hpp"

#include "scip/utils.hpp"

namespace ecole {
namespace scip {

Model::Model() : scimpl(std::make_unique<Scimpl>()) {}

Model::Model(Model&&) noexcept = default;

Model::Model(std::unique_ptr<Scimpl>&& other_scimpl) : scimpl(std::move(other_scimpl)) {}

Model::~Model() = default;

Model& Model::operator=(Model&&) noexcept = default;

SCIP* Model::get_scip_ptr() const noexcept {
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
	model.read_prob(filename);
	return model;
}

void Model::read_prob(std::string const& filename) {
	scip::call(SCIPreadProb, get_scip_ptr(), filename.c_str(), nullptr);
}

Stage Model::get_stage() const noexcept {
	return SCIPgetStage(get_scip_ptr());
}

ParamType Model::get_param_type(std::string const& name) const {
	auto* scip_param = SCIPgetParam(get_scip_ptr(), name.c_str());
	if (!scip_param)
		throw scip::Exception(fmt::format("parameter <{}> unknown", name));
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
			throw Exception(fmt::format("Could not find type for parameter '{}'", name));
		}
}

template <> void Model::set_param<ParamType::Bool>(std::string const& name, bool value) {
	scip::call(SCIPsetBoolParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::Int>(std::string const& name, int value) {
	scip::call(SCIPsetIntParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::LongInt>(std::string const& name, long_int value) {
	scip::call(SCIPsetLongintParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::Real>(std::string const& name, real value) {
	scip::call(SCIPsetRealParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::Char>(std::string const& name, char value) {
	scip::call(SCIPsetCharParam, get_scip_ptr(), name.c_str(), value);
}
template <> void Model::set_param<ParamType::String>(std::string const& name, std::string value) {
	scip::call(SCIPsetStringParam, get_scip_ptr(), name.c_str(), value.c_str());
}

template <> bool Model::get_param<ParamType::Bool>(std::string const& name) const {
	SCIP_Bool value{};
	scip::call(SCIPgetBoolParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <> int Model::get_param<ParamType::Int>(std::string const& name) const {
	int value{};
	scip::call(SCIPgetIntParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <> long_int Model::get_param<ParamType::LongInt>(std::string const& name) const {
	SCIP_Longint value{};
	scip::call(SCIPgetLongintParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <> real Model::get_param<ParamType::Real>(std::string const& name) const {
	SCIP_Real value{};
	scip::call(SCIPgetRealParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <> char Model::get_param<ParamType::Char>(std::string const& name) const {
	char value{};
	scip::call(SCIPgetCharParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <> std::string Model::get_param<ParamType::String>(std::string const& name) const {
	char* ptr{};
	scip::call(SCIPgetStringParam, get_scip_ptr(), name.c_str(), &ptr);
	return ptr;
}

void Model::set_params(std::map<std::string, Param> name_values) {
	for (auto&& name_val : name_values) {
		set_param(std::move(name_val.first), std::move(name_val.second));
	}
}

std::map<std::string, Param> Model::get_params() const {
	auto* const scip = get_scip_ptr();
	auto* const* const scip_params = SCIPgetParams(scip);
	auto const n_scip_params = SCIPgetNParams(scip);

	std::map<std::string, Param> params{};
	for (auto i = 0; i < n_scip_params; ++i) {
		std::string name = SCIPparamGetName(scip_params[i]);
		params.insert({std::move(name), get_param<Param>(name)});
	}
	return params;
}

void Model::solve() {
	scip::call(SCIPsolve, get_scip_ptr());
}

bool Model::is_solved() const noexcept {
	return SCIPgetStage(get_scip_ptr()) == SCIP_STAGE_SOLVED;
}

void Model::solve_iter() {
	scimpl->solve_iter();
}

void Model::solve_iter_branch(VarProxy var) {
	scimpl->solve_iter_branch(var.value);
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

VarView Model::variables() const noexcept {
	auto const scip_ptr = get_scip_ptr();
	auto const n_vars = static_cast<std::size_t>(SCIPgetNVars(scip_ptr));
	return VarView(scip_ptr, SCIPgetVars(scip_ptr), n_vars);
}

VarView Model::lp_branch_cands() const noexcept {
	int n_vars{};
	SCIP_VAR** vars{};
	scip::call(
		SCIPgetLPBranchCands, get_scip_ptr(), &vars, nullptr, nullptr, &n_vars, nullptr, nullptr);
	return VarView(get_scip_ptr(), vars, static_cast<std::size_t>(n_vars));
}

ColView Model::lp_columns() const {
	auto const scip_ptr = get_scip_ptr();
	if (SCIPgetStage(scip_ptr) != SCIP_STAGE_SOLVING)
		throw Exception("LP columns are only available during solving");
	auto const n_cols = static_cast<std::size_t>(SCIPgetNLPCols(scip_ptr));
	return ColView(scip_ptr, SCIPgetLPCols(scip_ptr), n_cols);
}

RowView Model::lp_rows() const {
	auto const scip_ptr = get_scip_ptr();
	if (SCIPgetStage(scip_ptr) != SCIP_STAGE_SOLVING)
		throw Exception("LP rows are only available during solving");
	auto const n_rows = static_cast<std::size_t>(SCIPgetNLPRows(scip_ptr));
	return RowView(scip_ptr, SCIPgetLPRows(scip_ptr), n_rows);
}

namespace internal {

template <> std::string Caster<std::string, char>::cast(char val) {
	return std::string{val};
}

template <> char Caster<char, char const*>::cast(char const* val) {
	if (strlen(val) == 1)
		return val[0];
	else
		throw scip::Exception("Can only convert a string with a single character to a char");
}
template <> char Caster<char, std::string>::cast(std::string val) {
	if (val.length() == 1)
		return val[0];
	else
		throw scip::Exception("Can only convert a string with a single character to a char");
}

}  // namespace internal

}  // namespace scip
}  // namespace ecole
