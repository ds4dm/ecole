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

template <typename T> struct Deleter { void operator()(T* ptr); };
template <typename T> using unique_ptr = std::unique_ptr<T, Deleter<T>>;

unique_ptr<Scip> create();

enum class ParamType { Bool, Int, LongInt, Real, Char, String };
namespace internal {
template <ParamType> struct ParamType_get;
template <> struct ParamType_get<ParamType::Bool> { using type = unsigned int; };
template <> struct ParamType_get<ParamType::Int> { using type = int; };
template <> struct ParamType_get<ParamType::LongInt> { using type = long long int; };
template <> struct ParamType_get<ParamType::Real> { using type = double; };
template <> struct ParamType_get<ParamType::Char> { using type = char; };
template <> struct ParamType_get<ParamType::String> { using type = const char*; };
}  // namespace internal
template <ParamType T> using param_t = typename internal::ParamType_get<T>::type;

class Model {
public:
	using BranchFunc = std::function<VarProxy(Model&)>;

	Model();
	Model(unique_ptr<Scip>&& scip);
	Model(Model const& model);
	Model& operator=(Model const&);
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;
	virtual ~Model() = default;

	static Model from_file(std::string const& filename);

	ParamType get_param_type(const char* name) const;
	template <typename T> void set_param_explicit(const char* name, T value);
	template <typename T> void set_param_explicit(std::string const& name, T value);
	template <typename T> void set_param(const char* name, T value);
	template <typename T> void set_param(std::string const& name, T value);
	template <typename T> T get_param_explicit(const char* name) const;
	template <typename T> T get_param_explicit(std::string const& name) const;
	template <typename T> T get_param(const char* name) const;
	template <typename T> T get_param(std::string const& name) const;

	void solve();
	void interrupt_solve();

	void disable_presolve();
	void disable_cuts();

	bool is_solved() const noexcept;

	VarView variables() const noexcept;
	VarView lp_branch_vars() const noexcept;

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
	operator bool() const { assert(false); }
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
	}
}

template <typename T> T Model::get_param(std::string const& name) const {
	return get_param<T>(name.c_str());
}

}  // namespace scip
}  // namespace ecole
