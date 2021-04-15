#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>

#include <nonstd/span.hpp>
#include <scip/scip.h>

#include "ecole/scip/exception.hpp"
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
class Model {
public:
	/**
	 * Construct an *initialized* model with default SCIP plugins.
	 */
	Model();
	Model(Model&& /*other*/) noexcept;
	Model(Model const& model) = delete;
	Model(std::unique_ptr<Scimpl>&& /*other_scimpl*/);

	~Model();

	Model& operator=(Model&& /*other*/) noexcept;
	Model& operator=(Model const&) = delete;

	/**
	 * Access the underlying SCIP pointer.
	 *
	 * Ownership of the pointer is however not released by the Model.
	 * This function is meant to use the original C API of SCIP.
	 */
	[[nodiscard]] SCIP* get_scip_ptr() noexcept;
	[[nodiscard]] SCIP const* get_scip_ptr() const noexcept;

	[[nodiscard]] Model copy_orig() const;

	/**
	 * Compare if two model share the same SCIP pointer, _i.e._ the same memory.
	 */
	bool operator==(Model const& other) const noexcept;
	bool operator!=(Model const& other) const noexcept;

	/**
	 * Construct a model by reading a problem file supported by SCIP (LP, MPS,...).
	 */
	static Model from_file(std::string const& filename);

	/**
	 * Constuct an empty problem with empty data structures.
	 */
	static Model prob_basic(std::string const& name = "Model");

	/**
	 * Writes the Model into a file.
	 */
	void write_problem(const std::string& filename) const;

	/**
	 * Read a problem file into the Model.
	 */
	void read_problem(std::string const& filename);

	[[nodiscard]] std::string name() const noexcept;
	void set_name(std::string const& name);

	[[nodiscard]] SCIP_STAGE get_stage() const noexcept;

	[[nodiscard]] ParamType get_param_type(std::string const& name) const;

	/**
	 * Get and set parameters by their exact SCIP type.
	 *
	 * The method will throw an exception if the type is not *exactly* the one used
	 * by SCIP.
	 */
	template <ParamType T> void set_param(std::string const& name, utility::value_or_const_ref_t<param_t<T>> value);
	template <ParamType T> [[nodiscard]] param_t<T> get_param(std::string const& name) const;

	/**
	 * Get and set parameters with automatic casting.
	 *
	 * Often, it is not required to know the exact type of a parameters to set its value
	 * (for instance when setting to zero).
	 * These methods do their best to convert to and from the required type.
	 */
	template <typename T> void set_param(std::string const& name, T value);
	template <typename T> [[nodiscard]] T get_param(std::string const& name) const;

	void set_params(std::map<std::string, Param> name_values);
	[[nodiscard]] std::map<std::string, Param> get_params() const;

	void disable_presolve();
	void disable_cuts();

	/**
	 * Transform, presolve, and solve problem.
	 */
	void transform_prob();
	void presolve();
	void solve();
	[[nodiscard]] bool is_solved() const noexcept;

	void solve_iter();
	void solve_iter_branch(SCIP_VAR* var);
	void solve_iter_stop();
	[[nodiscard]] bool solve_iter_is_done();

	[[nodiscard]] nonstd::span<SCIP_VAR*> variables() const noexcept;
	[[nodiscard]] nonstd::span<SCIP_VAR*> lp_branch_cands() const;
	[[nodiscard]] nonstd::span<SCIP_VAR*> pseudo_branch_cands() const;
	[[nodiscard]] nonstd::span<SCIP_COL*> lp_columns() const;
	[[nodiscard]] nonstd::span<SCIP_CONS*> constraints() const noexcept;
	[[nodiscard]] nonstd::span<SCIP_ROW*> lp_rows() const;
	[[nodiscard]] std::size_t nnz() const noexcept;

private:
	std::unique_ptr<Scimpl> scimpl;
};

/*****************************
 *  Implementation of Model  *
 *****************************/

namespace internal {

// SFINAE default class for no available cast
template <typename To, typename From, typename = void> struct Caster {
	static To cast(From /*unused*/) { throw Exception("Cannot convert to the desired type"); }
};

// SFINAE class for narrow cast
template <typename To, typename From>
struct Caster<To, From, std::enable_if_t<utility::is_narrow_castable_v<From, To>>> {
	static To cast(From val) { return utility::narrow_cast<To>(val); }
};

// SFINAE class for convertible but not narrowablecast
template <typename To, typename From>
struct Caster<To, From, std::enable_if_t<!utility::is_narrow_castable_v<From, To> && std::is_convertible_v<From, To>>> {
	static To cast(From val) { return static_cast<To>(val); }
};

// Visit From variants.
// Cannot static_cast a variant into one of its held value. Other way around works though.
template <typename To, typename... VariantFrom> struct Caster<To, std::variant<VariantFrom...>> {
	static To cast(std::variant<VariantFrom...> variant_val) {
		return std::visit([](auto val) { return Caster<To, decltype(val)>::cast(val); }, variant_val);
	}
};

// Pointers must not convert to bools
template <typename From> struct Caster<bool, std::remove_cv<From>*> {
	static bool cast(From /*unused*/) { throw Exception("Cannot convert pointers to bool"); }
};

// Convert character to string
template <> std::string Caster<std::string, char>::cast(char val);

// Convert string to character
template <> char Caster<char, char const*>::cast(char const* val);
template <> char Caster<char, std::string>::cast(std::string val);

// Helper func to deduce From type automatically
template <typename To, typename From> To cast(From val) {
	return Caster<To, From>::cast(val);
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
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception("Could not find type for given parameter");
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
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception("Could not find type for given parameter");
	}
}

}  // namespace ecole::scip
