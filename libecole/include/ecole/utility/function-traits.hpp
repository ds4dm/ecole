#pragma once

#include <cstddef>
#include <tuple>

namespace ecole::utility {

/**
 * Traits of functions, functions pointer, and function objects.
 *
 * Provide static access to:
 *   - `n_args` number of argument of the function.
 *   - `args` arguements types.
 *   - `return_type` the type of the function return.
 *
 * Implementation from https://functionalcpp.wordpress.com/2013/08/05/function-traits/
 */
template <typename F> struct function_traits;

template <typename Return, typename... Args> struct function_traits<Return(Args...)> {
	using return_type = Return;

	static constexpr std::size_t n_args = sizeof...(Args);

	template <std::size_t N> struct args {
		static_assert(N < n_args, "error: invalid parameter index.");
		using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};
};

/**
 * Specialization for function pointers.
 */
template <typename Return, typename... Args>
struct function_traits<Return (*)(Args...)> : function_traits<Return(Args...)> {};

/**
 * Specialization for member function pointers.
 */
template <typename Class, typename Return, typename... Args>
struct function_traits<Return (Class::*)(Args...)> : public function_traits<Return(Class&, Args...)> {};

/**
 * Specialization for const member function pointer.
 */
template <typename Class, typename Return, typename... Args>
struct function_traits<Return (Class::*)(Args...) const> : function_traits<Return(Class&, Args...)> {};

/**
 * Specialization for member object pointers.
 */
template <typename Class, typename Return>
struct function_traits<Return(Class::*)> : function_traits<Return(Class&)> {};

/**
 * Specialization for functors.
 */
template <typename F> struct function_traits {
private:
	using call_type = function_traits<decltype(&F::type::operator())>;

public:
	using return_type = typename call_type::return_type;

	static constexpr std::size_t n_args = call_type::n_args - 1;

	template <std::size_t N> struct args {
		static_assert(N < n_args, "error: invalid parameter index.");
		using type = typename call_type::template args<N + 1>::type;
	};
};

/**
 * Specialization for ref functors.
 */
template <typename F> struct function_traits<F&> : public function_traits<F> {};

/**
 * Specialization for rvalue ref functors.
 */
template <typename F> struct function_traits<F&&> : public function_traits<F> {};

/**
 * Helper alias for return type.
 */
template <typename F> using return_t = typename function_traits<F>::return_type;

/**
 * Helper type for argument type.
 */
template <std::size_t N, typename F> using arg_t = typename function_traits<F>::template args<N>::type;

}  // namespace ecole::utility
