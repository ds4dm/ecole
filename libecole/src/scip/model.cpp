#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <mutex>
#include <string>

#include <fmt/format.h>
#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/exception.hpp"
#include "ecole/scip/model.hpp"

#include "scip/utils.hpp"

namespace ecole {
namespace scip {

template <> void Deleter<SCIP>::operator()(SCIP* scip) {
	scip::call(SCIPfree, &scip);
}

unique_ptr<SCIP> create() {
	SCIP* scip_raw;
	scip::call(SCIPcreate, &scip_raw);
	SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip_raw), true);
	auto scip_ptr = unique_ptr<SCIP>{};
	scip_ptr.reset(scip_raw);
	return scip_ptr;
}

unique_ptr<SCIP> copy(SCIP const* source) {
	if (!source) return nullptr;
	if (SCIPgetStage(const_cast<SCIP*>(source)) == SCIP_STAGE_INIT) return create();
	auto dest = create();
	// Copy operation is not thread safe
	static std::mutex m{};
	std::lock_guard<std::mutex> g{m};
	scip::call(
		SCIPcopy,
		const_cast<SCIP*>(source),
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

SCIP* Model::get_scip_ptr() const noexcept {
	return scip.get();
}

Model::Model() : scip(create()) {
	scip::call(SCIPincludeDefaultPlugins, get_scip_ptr());
}

Model::Model(unique_ptr<SCIP>&& scip) {
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
			throw Exception(fmt::format("Could not find type for parameter '{}'", name));
		}
}

template <>
void Model::set_param_explicit<ParamType::Bool>(std::string const& name, bool value) {
	scip::call(SCIPsetBoolParam, get_scip_ptr(), name.c_str(), value);
}
template <>
void Model::set_param_explicit<ParamType::Int>(std::string const& name, int value) {
	scip::call(SCIPsetIntParam, get_scip_ptr(), name.c_str(), value);
}
template <>
void Model::set_param_explicit<ParamType::LongInt>(
	std::string const& name,
	long_int value) {
	scip::call(SCIPsetLongintParam, get_scip_ptr(), name.c_str(), value);
}
template <>
void Model::set_param_explicit<ParamType::Real>(std::string const& name, real value) {
	scip::call(SCIPsetRealParam, get_scip_ptr(), name.c_str(), value);
}
template <>
void Model::set_param_explicit<ParamType::Char>(std::string const& name, char value) {
	scip::call(SCIPsetCharParam, get_scip_ptr(), name.c_str(), value);
}
template <>
void Model::set_param_explicit<ParamType::String>(
	std::string const& name,
	std::string const& value) {
	scip::call(SCIPsetStringParam, get_scip_ptr(), name.c_str(), value.c_str());
}

template <>
bool Model::get_param_explicit<ParamType::Bool>(std::string const& name) const {
	SCIP_Bool value{};
	scip::call(SCIPgetBoolParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <> int Model::get_param_explicit<ParamType::Int>(std::string const& name) const {
	int value{};
	scip::call(SCIPgetIntParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <>
long_int Model::get_param_explicit<ParamType::LongInt>(std::string const& name) const {
	SCIP_Longint value{};
	scip::call(SCIPgetLongintParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <>
real Model::get_param_explicit<ParamType::Real>(std::string const& name) const {
	SCIP_Real value{};
	scip::call(SCIPgetRealParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <>
char Model::get_param_explicit<ParamType::Char>(std::string const& name) const {
	char value{};
	scip::call(SCIPgetCharParam, get_scip_ptr(), name.c_str(), &value);
	return value;
}
template <>
std::string Model::get_param_explicit<ParamType::String>(std::string const& name) const {
	char* ptr{};
	scip::call(SCIPgetStringParam, get_scip_ptr(), name.c_str(), &ptr);
	return ptr;
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
	auto const scip_ptr = get_scip_ptr();
	auto const n_vars = static_cast<std::size_t>(SCIPgetNVars(scip_ptr));
	return VarView(scip_ptr, SCIPgetVars(scip_ptr), n_vars);
}

VarView Model::lp_branch_cands() const noexcept {
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

static auto matrix_size(Model const& model) {
	std::size_t nnz = 0;
	for (auto row : model.lp_rows())
		nnz += static_cast<std::size_t>(row.n_lp_nonz());
	return nnz;
}

utility::coo_matrix<real> Model::lp_matrix() const {
	auto const scip = get_scip_ptr();
	if (SCIPgetStage(scip) != SCIP_STAGE_SOLVING)
		throw Exception("LP matrix are only available during solving");

	using coo_matrix = utility::coo_matrix<real>;
	auto const nnz = matrix_size(*this);
	auto values = decltype(coo_matrix::values)::from_shape({nnz});
	auto indices = decltype(coo_matrix::indices)::from_shape({2, nnz});

	auto n_rows = static_cast<std::size_t>(SCIPgetNLPRows(scip));
	auto n_cols = static_cast<std::size_t>(SCIPgetNLPCols(scip));

	SCIP_ROW** const rows = SCIPgetLPRows(scip);

	for (std::size_t i = 0, j = 0; i < n_rows; ++i) {
		SCIP_COL** const row_cols = SCIProwGetCols(rows[i]);
		real const* const row_vals = SCIProwGetVals(rows[i]);
		std::size_t const row_nnz = static_cast<std::size_t>(SCIProwGetNLPNonz(rows[i]));
		for (std::size_t k = 0; k < row_nnz; ++k) {
			indices(0, j + k) = i;
			indices(1, j + k) = static_cast<std::size_t>(SCIPcolGetLPPos(row_cols[k]));
			values[j + k] = row_vals[k];
		}
		j += row_nnz;
	}

	return {values, indices, {n_rows, n_cols}};
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
