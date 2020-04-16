#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include <fmt/format.h>

#include <scip/scip.h>

#include "ecole/scip/column.hpp"
#include "ecole/scip/row.hpp"
#include "ecole/scip/utils.hpp"
#include "ecole/scip/variable.hpp"
#include "ecole/utility/type_traits.hpp"

namespace ecole {
namespace scip {

/**
 * Wrap SCIP pointer free function in a deleter for use with smart pointers.
 */
template <typename T> struct Deleter { void operator()(T* ptr); };
template <typename T> using unique_ptr = std::unique_ptr<T, Deleter<T>>;

/**
 * Create an initialized SCIP pointer without message handler.
 */
unique_ptr<SCIP> create();

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
	Model(unique_ptr<SCIP>&& scip);
	/**
	 * Deep copy the model.
	 */
	Model(Model const& model);
	Model& operator=(Model const&);
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;
	~Model() = default;

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
	 * Read a problem file into the Model.
	 */
	void read_prob(std::string const& filename);

	Stage get_stage() const noexcept;

	ParamType get_param_type(std::string const& name) const;

	/**
	 * Get and set parameters by their exact SCIP type.
	 *
	 * The method will throw an exception if the type is not *exactly* the one used
	 * by SCIP.
	 *
	 * @see get_param, set_param to convert automatically.
	 */
	template <ParamType T, std::enable_if_t<T != ParamType::String, int> = 0>
	void set_param_explicit(std::string const& name, param_t<T> value);
	template <ParamType T, std::enable_if_t<T == ParamType::String, int> = 0>
	void set_param_explicit(std::string const& name, std::string const& value);
	template <ParamType T> param_t<T> get_param_explicit(std::string const& name) const;

	/**
	 * Get and set parameters with automatic casting.
	 *
	 * Often, it is not required to know the exact type of a parameters to set its value
	 * (for instance when setting to zero).
	 * These methods do their best to convert to and from the required type.
	 *
	 * @see get_param_explicit, set_param_explicit to avoid any conversions.
	 */
	// setter specialization for strings
	void set_param(std::string const& name, std::string const& value);
	// setter specialization for arithmetic types
	template <typename T>
	typename std::enable_if<std::is_arithmetic<typename std::decay<T>::type>::value, void>::
		type
		set_param(std::string const& name, T value);
	// getter specialization for strings
	template <typename T>
	typename std::
		enable_if<std::is_same<typename std::decay<T>::type, std::string>::value, T>::type
		get_param(std::string const& name) const;
	// getter specialization for arithmetic types
	template <typename T>
	typename std::enable_if<std::is_arithmetic<typename std::decay<T>::type>::value, T>::
		type
		get_param(std::string const& name) const;

	void disable_presolve();
	void disable_cuts();

	/**
	 * Transform, presolve, and solve problem.
	 */
	void solve();
	void interrupt_solve();

	bool is_solved() const noexcept;

	VarView variables() const noexcept;
	VarView lp_branch_cands() const noexcept;
	ColView lp_columns() const;
	RowView lp_rows() const;

	/**
	 * Access the underlying SCIP pointer.
	 *
	 * Ownership of the pointer is however not released by the Model.
	 * This function is meant to use the original C API of SCIP.
	 */
	SCIP* get_scip_ptr() const noexcept;

private:
	unique_ptr<SCIP> scip;
};

/*****************************
 *  Implementation of Model  *
 *****************************/

namespace internal {

template <class To, class From>
typename std::enable_if<std::is_same<To, From>::value == true, To>::type
narrow_cast(From v) {
	return v;
}

template <class To, class From>
typename std::enable_if<std::is_same<To, From>::value == false, To>::type
narrow_cast(From v) {
	auto v_to = static_cast<To>(v);
	auto v_back = static_cast<From>(v_to);
	if (v_back != v) {
		throw Exception(
			fmt::format("Narrow cast failed: numerical loss from '{}' to '{}'", v, v_to));
	}
	return v_to;
}

}  // namespace internal

// set_param() specialization for arithmetic types
template <typename T>
typename std::enable_if<std::is_arithmetic<typename std::decay<T>::type>::value, void>::
	type
	Model::set_param(std::string const& name, T value) {
	using namespace internal;
	switch (get_param_type(name)) {
	case ParamType::Bool:
		set_param_explicit<ParamType::Bool>(
			name, narrow_cast<param_t<ParamType::Bool>>(value));
		break;
	case ParamType::Int:
		set_param_explicit<ParamType::Int>(name, narrow_cast<param_t<ParamType::Int>>(value));
		break;
	case ParamType::LongInt:
		set_param_explicit<ParamType::LongInt>(
			name, narrow_cast<param_t<ParamType::LongInt>>(value));
		break;
	case ParamType::Real:
		set_param_explicit<ParamType::Real>(
			name, narrow_cast<param_t<ParamType::Real>>(value));
		break;
	case ParamType::Char:
		set_param_explicit<ParamType::Char>(
			name, narrow_cast<param_t<ParamType::Char>>(value));
		break;
	case ParamType::String:
		throw Exception(fmt::format("Parameter {} does not accept numeric values", name));
	default:
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception(fmt::format("Could not find type for parameter {}", name));
	}
}

// get_param() specialization for string types
template <typename T>
typename std::
	enable_if<std::is_same<typename std::decay<T>::type, std::string>::value, T>::type
	Model::get_param(std::string const& name) const {
	using namespace internal;
	switch (get_param_type(name)) {
	case ParamType::Bool:
	case ParamType::Int:
	case ParamType::LongInt:
	case ParamType::Real:
		throw Exception(
			fmt::format("Parameter {} does not export into a string value", name));
	case ParamType::Char:
		return std::string{1, get_param_explicit<ParamType::Char>(name)};
	case ParamType::String:
		return get_param_explicit<ParamType::String>(name);
	default:
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception(fmt::format("Could not find type for parameter {}", name));
	}
}

// get_param() specialization for arithmetic types
template <typename T>
typename std::enable_if<std::is_arithmetic<typename std::decay<T>::type>::value, T>::type
Model::get_param(std::string const& name) const {
	using namespace internal;
	switch (get_param_type(name)) {
	case ParamType::Bool:
		return narrow_cast<T>(get_param_explicit<ParamType::Bool>(name));
	case ParamType::Int:
		return narrow_cast<T>(get_param_explicit<ParamType::Int>(name));
	case ParamType::LongInt:
		return narrow_cast<T>(get_param_explicit<ParamType::LongInt>(name));
	case ParamType::Real:
		return narrow_cast<T>(get_param_explicit<ParamType::Real>(name));
	case ParamType::Char:
		return narrow_cast<T>(get_param_explicit<ParamType::Char>(name));
	case ParamType::String:
		throw Exception(
			fmt::format("Parameter {} does not export into a numeric value", name));
	default:
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception(fmt::format("Could not find type for parameter {}", name));
	}
}

}  // namespace scip
}  // namespace ecole
