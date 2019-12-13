#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include "ecole/scip/variable.hpp"

// Avoid including SCIP header
typedef struct Scip Scip;

namespace ecole {
namespace scip {

/**
 * Wrap Scip pointer free function in a deleter for use with smart pointers.
 */
template <typename T> struct Deleter { void operator()(T* ptr); };
template <typename T> using unique_ptr = std::unique_ptr<T, Deleter<T>>;

/**
 * Create an initialized Scip pointer without message handler.
 */
unique_ptr<Scip> create();

/**
 * Types of parameters supported by Scip.
 *
 * @see param_t to get the associated type.
 */
enum class ParamType { Bool, Int, LongInt, Real, Char, String };

namespace internal {
// Use with `param_t`.
// File `model.cpp` contains `static_assert`s to ensure this is never out of date
// with Scip internals.
template <ParamType> struct ParamType_get;
template <> struct ParamType_get<ParamType::Bool> { using type = unsigned int; };
template <> struct ParamType_get<ParamType::Int> { using type = int; };
template <> struct ParamType_get<ParamType::LongInt> { using type = long long int; };
template <> struct ParamType_get<ParamType::Real> { using type = double; };
template <> struct ParamType_get<ParamType::Char> { using type = char; };
template <> struct ParamType_get<ParamType::String> { using type = const char*; };
}  // namespace internal

/**
 * Type associated with a ParamType.
 */
template <ParamType T> using param_t = typename internal::ParamType_get<T>::type;

/**
 * A stateful Scip solver object.
 *
 * A RAII class to manage an underlying `SCIP*`.
 * This is somehow similar to a `pyscipopt.Model`, but with higher level methods
 * tailored for the needs in Ecole.
 * This is the only interface to Scip in the library.
 */
class Model {
public:
	/**
	 * Construct an *initialized* model with default Scip plugins.
	 */
	Model();
	Model(unique_ptr<Scip>&& scip);
	/**
	 * Deep copy the model.
	 */
	Model(Model const& model);
	Model& operator=(Model const&);
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;
	~Model() = default;

	/**
	 * Construct a model by reading a problem file supported by Scip (LP, MPS,...).
	 */
	static Model from_file(std::string const& filename);

	ParamType get_param_type(const char* name) const;
	ParamType get_param_type(std::string const& name) const;

	/**
	 * Get and set parameters by their exact Scip type.
	 *
	 * The method will throw an exception if the type is not *exactly* the one used
	 * by Scip.
	 *
	 * @see get_param, set_param to convert automatically.
	 */
	template <typename T> void set_param_explicit(const char* name, T value);
	template <typename T> void set_param_explicit(std::string const& name, T value);
	template <typename T> T get_param_explicit(const char* name) const;
	template <typename T> T get_param_explicit(std::string const& name) const;

	/**
	 * Get and set parameters with automatic casting.
	 *
	 * Often, it is not required to know the exact type of a parameters to set its value
	 * (for instance when setting to zero).
	 * These methods do their best to convert to and from the required type.
	 *
	 * @see get_param_explicit, set_param_explicit to avoid any conversions.
	 */
	template <typename T> void set_param(const char* name, T value);
	template <typename T> void set_param(std::string const& name, T value);
	template <typename T> T get_param(const char* name) const;
	template <typename T> T get_param(std::string const& name) const;

	/**
	 * Get the current random seed of the Model.
	 */
	param_t<ParamType::Int> seed() const;
	/**
	 * Set the Model random seed shift.
	 *
	 * Set the shift used by with all random seeds in Scip.
	 * Random seed for individual compenents of Scip can be set throught the parameters
	 * but will nontheless be shifted by the value set here.
	 * Set a value of zero to disable shiftting.
	 */
	void seed(param_t<ParamType::Int> seed_v);

	void disable_presolve();
	void disable_cuts();

	/**
	 * Transform, presolve, and solve problem.
	 */
	void solve();
	void interrupt_solve();

	bool is_solved() const noexcept;

	VarView variables() const noexcept;
	VarView lp_branch_vars() const noexcept;

	using BranchFunc = std::function<VarProxy(Model&)>;
	void set_branch_rule(BranchFunc const& func);

private:
	class LambdaBranchRule;
	unique_ptr<Scip> scip;
};

namespace internal {

// SFINAE to check if type exists
template <typename> struct exists { using type = void; };
template <typename T> using exists_t = typename exists<T>::type;
// Helper to check static cast ability
template <typename To, typename From>
using can_cast_t = exists_t<decltype(static_cast<To>(std::declval<From>()))>;

// SFINAE default class for no available cast
template <typename To, typename From, typename = void> struct Cast_SFINAE {
	From val;
	operator To() const { throw Exception("Cannot convert to the desired type"); }
};

// SFINAE for available cast
template <typename To, typename From> struct Cast_SFINAE<To, From, can_cast_t<To, From>> {
	From val;
	operator To() const { return static_cast<To>(val); }
};

// Pointers must not convert to bools
template <typename From> struct Cast_SFINAE<bool, From*> {
	From val;
	operator bool() const { throw Exception("Cannot convert to the desired type"); }
};

// C-string can be converted to char if single character
template <> Cast_SFINAE<char, const char*>::operator char() const;

// Helper func to deduce From type automatically
template <typename To, typename From> To cast(From x) {
	return Cast_SFINAE<To, From>{x};
}

}  // namespace internal

template <typename T> void Model::set_param_explicit(std::string const& name, T value) {
	return set_param_explicit(name.c_str(), value);
}

template <typename T> void Model::set_param(const char* name, T value) {
	using namespace internal;
	switch (get_param_type(name)) {
	case ParamType::Bool:
		return set_param_explicit(name, cast<param_t<ParamType::Bool>>(value));
	case ParamType::Int:
		return set_param_explicit(name, cast<param_t<ParamType::Int>>(value));
	case ParamType::LongInt:
		return set_param_explicit(name, cast<param_t<ParamType::LongInt>>(value));
	case ParamType::Real:
		return set_param_explicit(name, cast<param_t<ParamType::Real>>(value));
	case ParamType::Char:
		return set_param_explicit(name, cast<param_t<ParamType::Char>>(value));
	case ParamType::String:
		return set_param_explicit(name, cast<param_t<ParamType::String>>(value));
	default:
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception("Could not find type for given parameter");
	}
}

template <> void Model::set_param(const char* name, std::string const& value);

template <typename T> void Model::set_param(std::string const& name, T value) {
	return set_param(name.c_str(), value);
}

template <typename T> T Model::get_param_explicit(std::string const& name) const {
	return get_param_explicit<T>(name.c_str());
}

template <typename T> T Model::get_param(const char* name) const {
	using namespace internal;
	switch (get_param_type(name)) {
	case ParamType::Bool:
		return cast<T>(get_param_explicit<param_t<ParamType::Bool>>(name));
	case ParamType::Int:
		return cast<T>(get_param_explicit<param_t<ParamType::Int>>(name));
	case ParamType::LongInt:
		return cast<T>(get_param_explicit<param_t<ParamType::LongInt>>(name));
	case ParamType::Real:
		return cast<T>(get_param_explicit<param_t<ParamType::Real>>(name));
	case ParamType::Char:
		return cast<T>(get_param_explicit<param_t<ParamType::Char>>(name));
	case ParamType::String:
		return cast<T>(get_param_explicit<param_t<ParamType::String>>(name));
	default:
		assert(false);  // All enum value should be handled
		// Non void return for optimized build
		throw Exception("Could not find type for given parameter");
	}
}

template <typename T> T Model::get_param(std::string const& name) const {
	return get_param<T>(name.c_str());
}

}  // namespace scip
}  // namespace ecole
