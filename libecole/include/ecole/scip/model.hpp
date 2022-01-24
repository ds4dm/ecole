#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <nonstd/span.hpp>
#include <scip/scip.h>

#include "ecole/export.hpp"
#include "ecole/scip/exception.hpp"
#include "ecole/scip/stop-location.hpp"
#include "ecole/scip/type.hpp"
#include "ecole/utility/numeric.hpp"
#include "ecole/utility/type-traits.hpp"

namespace ecole::scip {

/* Forward declare scip holder type */
class Scimpl;

/**
 * A stateful SCIP solver object.
 *
 * A RAII class to manage an underlying `SCIP*`.
 * This is somehow similar to a `pyscipopt.Model`, but with higher level methods
 * tailored for the needs in Ecole.
 * This is the only interface to SCIP in the library.
 */
class ECOLE_EXPORT Model {
public:
	/**
	 * Construct an *initialized* model with default SCIP plugins.
	 */
	ECOLE_EXPORT Model();
	ECOLE_EXPORT Model(Model&& /*other*/) noexcept;
	Model(Model const& model) = delete;
	ECOLE_EXPORT Model(std::unique_ptr<Scimpl>&& /*other_scimpl*/);

	ECOLE_EXPORT ~Model();

	ECOLE_EXPORT Model& operator=(Model&& /*other*/) noexcept;
	ECOLE_EXPORT Model& operator=(Model const&) = delete;

	/**
	 * Access the underlying SCIP pointer.
	 *
	 * Ownership of the pointer is however not released by the Model.
	 * This function is meant to use the original C API of SCIP.
	 */
	[[nodiscard]] ECOLE_EXPORT SCIP* get_scip_ptr() noexcept;
	[[nodiscard]] ECOLE_EXPORT SCIP const* get_scip_ptr() const noexcept;

	[[nodiscard]] ECOLE_EXPORT Model copy() const;
	[[nodiscard]] ECOLE_EXPORT Model copy_orig() const;

	/**
	 * Compare if two model share the same SCIP pointer, _i.e._ the same memory.
	 */
	ECOLE_EXPORT bool operator==(Model const& other) const noexcept;
	ECOLE_EXPORT bool operator!=(Model const& other) const noexcept;

	/**
	 * Construct a model by reading a problem file supported by SCIP (LP, MPS,...).
	 */
	ECOLE_EXPORT static Model from_file(std::filesystem::path const& filename);

	/**
	 * Constuct an empty problem with empty data structures.
	 */
	ECOLE_EXPORT static Model prob_basic(std::string const& name = "Model");

	/**
	 * Writes the Model into a file.
	 */
	ECOLE_EXPORT void write_problem(std::filesystem::path const& filename) const;

	/**
	 * Read a problem file into the Model.
	 */
	ECOLE_EXPORT void read_problem(std::string const& filename);

	/**
	 * Change whether or not to write logging messages in the logger.
	 */
	ECOLE_EXPORT void set_messagehdlr_quiet(bool quiet) noexcept;

	[[nodiscard]] ECOLE_EXPORT std::string name() const noexcept;
	ECOLE_EXPORT void set_name(std::string const& name);

	[[nodiscard]] ECOLE_EXPORT SCIP_STAGE stage() const noexcept;

	[[nodiscard]] ECOLE_EXPORT ParamType get_param_type(std::string const& name) const;

	/**
	 * Get and set parameters by their exact SCIP type.
	 *
	 * The method will throw an exception if the type is not *exactly* the one used
	 * by SCIP.
	 */
	template <ParamType T>
	ECOLE_EXPORT void set_param(std::string const& name, utility::value_or_const_ref_t<param_t<T>> value);
	template <ParamType T> [[nodiscard]] ECOLE_EXPORT param_t<T> get_param(std::string const& name) const;

	/**
	 * Get and set parameters with automatic casting.
	 *
	 * Often, it is not required to know the exact type of a parameters to set its value
	 * (for instance when setting to zero).
	 * These methods do their best to convert to and from the required type.
	 */
	template <typename T> void set_param(std::string const& name, T value);
	template <typename T> [[nodiscard]] T get_param(std::string const& name) const;

	ECOLE_EXPORT void set_params(std::map<std::string, Param> name_values);
	[[nodiscard]] ECOLE_EXPORT std::map<std::string, Param> get_params() const;

	ECOLE_EXPORT void disable_presolve();
	ECOLE_EXPORT void disable_cuts();

	[[nodiscard]] ECOLE_EXPORT nonstd::span<SCIP_VAR*> variables() const noexcept;
	[[nodiscard]] ECOLE_EXPORT nonstd::span<SCIP_VAR*> lp_branch_cands() const;
	[[nodiscard]] ECOLE_EXPORT nonstd::span<SCIP_VAR*> pseudo_branch_cands() const;
	[[nodiscard]] ECOLE_EXPORT nonstd::span<SCIP_COL*> lp_columns() const;
	[[nodiscard]] ECOLE_EXPORT nonstd::span<SCIP_CONS*> constraints() const noexcept;
	[[nodiscard]] ECOLE_EXPORT nonstd::span<SCIP_ROW*> lp_rows() const;
	[[nodiscard]] ECOLE_EXPORT std::size_t nnz() const noexcept;

	/**
	 * Transform, presolve, and solve problem.
	 */
	ECOLE_EXPORT void transform_prob();
	ECOLE_EXPORT void presolve();
	ECOLE_EXPORT void solve();

	[[nodiscard]] ECOLE_EXPORT bool is_solved() const noexcept;
	[[nodiscard]] ECOLE_EXPORT SCIP_Real primal_bound() const noexcept;
	[[nodiscard]] ECOLE_EXPORT SCIP_Real dual_bound() const noexcept;

	ECOLE_EXPORT auto solve_iter(nonstd::span<DynamicCallbackConstructor const> arg_packs) -> std::optional<Callback>;
	ECOLE_EXPORT auto solve_iter(DynamicCallbackConstructor arg_pack) -> std::optional<Callback>;
	ECOLE_EXPORT auto solve_iter_continue(SCIP_RESULT result) -> std::optional<Callback>;

private:
	std::unique_ptr<Scimpl> scimpl;
};

/*****************************
 *  Implementation of Model  *
 *****************************/

template <> ECOLE_EXPORT void Model::set_param<ParamType::Bool>(std::string const& name, bool value);
template <> ECOLE_EXPORT void Model::set_param<ParamType::Int>(std::string const& name, int value);
template <> ECOLE_EXPORT void Model::set_param<ParamType::LongInt>(std::string const& name, SCIP_Longint value);
template <> ECOLE_EXPORT void Model::set_param<ParamType::Real>(std::string const& name, SCIP_Real value);
template <> ECOLE_EXPORT void Model::set_param<ParamType::Char>(std::string const& name, char value);
template <> ECOLE_EXPORT void Model::set_param<ParamType::String>(std::string const& name, std::string const& value);

template <> ECOLE_EXPORT auto Model::get_param<ParamType::Bool>(std::string const& name) const -> bool;
template <> ECOLE_EXPORT auto Model::get_param<ParamType::Int>(std::string const& name) const -> int;
template <> ECOLE_EXPORT auto Model::get_param<ParamType::LongInt>(std::string const& name) const -> SCIP_Longint;
template <> ECOLE_EXPORT auto Model::get_param<ParamType::Real>(std::string const& name) const -> SCIP_Real;
template <> ECOLE_EXPORT auto Model::get_param<ParamType::Char>(std::string const& name) const -> char;
template <> ECOLE_EXPORT auto Model::get_param<ParamType::String>(std::string const& name) const -> std::string;

namespace internal {

/**
 * Safely cast between various type, throwing an exception when impossible.
 */
template <typename To, typename From> auto cast([[maybe_unused]] From val) -> To {
	if constexpr (std::is_pointer_v<From> && std::is_same_v<To, bool>) {
		// Fallthrough to error. Don't convert pointers to bool.
	} else if constexpr (utility::is_narrow_castable_v<From, To>) {
		return utility::narrow_cast<To>(std::move(val));
	} else if constexpr (std::is_convertible_v<From, To>) {
		return static_cast<To>(val);
	} else if constexpr (utility::is_variant_v<From>) {
		return std::visit([](auto v) { return cast<To>(v); }, val);
	} else if constexpr (std::is_same_v<From, char> && std::is_same_v<To, std::string>) {
		return std::string{val};
	} else if constexpr (std::is_same_v<To, char>) {
		if constexpr (std::is_convertible_v<From, std::string_view>) {
			if (auto const str = std::string_view{val}; str.length() == 1) {
				return str[0];
			}
			// Fallthrough to error. Don't convert long string to char.
		}
	}
	throw ScipError::from_retcode(SCIP_PARAMETERWRONGTYPE);
}

}  // namespace internal

template <typename T> void Model::set_param(std::string const& name, T value) {
	using internal::cast;
	switch (get_param_type(name)) {
	case ParamType::Bool:
		return set_param<ParamType::Bool>(name, cast<bool>(value));
	case ParamType::Int:
		return set_param<ParamType::Int>(name, cast<int>(value));
	case ParamType::LongInt:
		return set_param<ParamType::LongInt>(name, cast<SCIP_Longint>(value));
	case ParamType::Real:
		return set_param<ParamType::Real>(name, cast<SCIP_Real>(value));
	case ParamType::Char:
		return set_param<ParamType::Char>(name, cast<char>(value));
	case ParamType::String:
		return set_param<ParamType::String>(name, cast<std::string>(value));
	default:
		// All enum value should be handled
		assert(false);
		// Non void return for optimized build
		throw ScipError::from_retcode(SCIP_PARAMETERUNKNOWN);
	}
}

template <typename T> T Model::get_param(std::string const& name) const {
	using namespace internal;
	switch (get_param_type(name)) {
	case ParamType::Bool:
		return cast<T>(get_param<ParamType::Bool>(name));
	case ParamType::Int:
		return cast<T>(get_param<ParamType::Int>(name));
	case ParamType::LongInt:
		return cast<T>(get_param<ParamType::LongInt>(name));
	case ParamType::Real:
		return cast<T>(get_param<ParamType::Real>(name));
	case ParamType::Char:
		return cast<T>(get_param<ParamType::Char>(name));
	case ParamType::String:
		return cast<T>(get_param<ParamType::String>(name));
	default:
		// All enum value should be handled
		assert(false);
		// Non void return for optimized build
		throw ScipError::from_retcode(SCIP_PARAMETERUNKNOWN);
	}
}

}  // namespace ecole::scip
