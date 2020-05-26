#pragma once

#include <cstddef>
#include <tuple>

namespace ecole {
namespace utility {

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

/**
 * Specialization for function pointers.
 */
template <typename R, typename... Args>
struct function_traits<R (*)(Args...)> : public function_traits<R(Args...)> {};

template <typename R, typename... Args> struct function_traits<R(Args...)> {
	using return_type = R;

	static constexpr std::size_t n_args = sizeof...(Args);

	template <std::size_t N> struct args {
		static_assert(N < n_args, "error: invalid parameter index.");
		using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};
};

/**
 * Specialization for member function pointers.
 */
template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : public function_traits<R(C&, Args...)> {};

/**
 * Specialization for const member function pointer.
 */
template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const> :
	public function_traits<R(C&, Args...)> {};

/**
 * Specialization for member object pointers.
 */
template <typename C, typename R>
struct function_traits<R(C::*)> : public function_traits<R(C&)> {};

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
template <std::size_t N, typename F>
using arg_t = typename function_traits<F>::template args<N>::type;

}  // namespace utility
}  // namespace ecole
