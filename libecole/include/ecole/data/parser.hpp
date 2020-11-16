#pragma once

#include <algorithm>
#include <iterator>
#include <map>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "ecole/data/constant.hpp"
#include "ecole/data/map.hpp"
#include "ecole/data/tuple.hpp"
#include "ecole/data/vector.hpp"
#include "ecole/traits.hpp"

namespace ecole::data {

template <typename Function> constexpr auto parse(Function func) noexcept {
	if constexpr (trait::is_data_function_v<Function>) {
		return func;
	} else {
		return ConstantFunction{std::move(func)};
	}
}

template <typename... Functions> constexpr auto parse(std::tuple<Functions...> func_tuple) {
	return std::apply([](auto&&... funcs) { return TupleFunction{parse(funcs)...}; }, std::move(func_tuple));
}

template <typename Function, typename Allocator> auto parse(std::vector<Function, Allocator> funcs) {
	using ParsedFunction = decltype(parse(std::declval<Function>()));
	if constexpr (std::is_same_v<Function, ParsedFunction>) {
		return VectorFunction{std::move(funcs)};
	} else {
		auto parsed_funcs = std::vector<ParsedFunction>(funcs.size());
		std::transform(
			std::move_iterator{funcs.begin()}, std::move_iterator{funcs.end()}, parsed_funcs.begin(), [](Function&& func) {
				return parse(std::move(func));
			});
		return parsed_funcs;
	}
}

template <typename Key, typename Function, typename Compare, typename Allocator>
auto parse(std::map<Key, Function, Compare, Allocator> funcs) {
	using ParsedFunction = decltype(parse(std::declval<Function>()));
	if constexpr (std::is_same_v<Function, ParsedFunction>) {
		return MapFunction{std::move(funcs)};
	} else {
		auto parsed_funcs = std::map<Key, ParsedFunction>{};
		std::for_each(std::move_iterator{funcs.begin()}, std::move_iterator{funcs.end()}, [&parsed_funcs](auto&& key_func) {
			auto [key, func] = std::forward<decltype(key_func)>(key_func);
			parsed_funcs.emplace(std::move(key), parse(std::move(func)));
		});
		return parsed_funcs;
	}
}

}  // namespace ecole::data
