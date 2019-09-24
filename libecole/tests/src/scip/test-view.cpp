#include <cstddef>
#include <iterator>
#include <list>
#include <memory>

#include <catch2/catch.hpp>

#include "ecole/scip/view.hpp"

using namespace ecole;

template <typename T> auto arange(std::size_t size) {
	auto deleter = [size](T const* const* data) {
		for (std::size_t i = 0; i < size; ++i)
			delete data[i];
		delete[] data;
	};
	auto make_data = [size] {
		auto data = new T*[size];
		for (std::size_t i = 0; i < size; ++i)
			data[i] = new T{static_cast<T>(i)};
		return data;
	};
	return std::unique_ptr<T const* const, decltype(deleter)>(make_data(), deleter);
}

TEMPLATE_TEST_CASE("Create a view", "", int, double) {
	struct Proxy : public scip::Proxy<TestType> {
		Proxy(TestType const* value) noexcept : scip::Proxy<TestType>(value) {}
		TestType times(TestType n) const noexcept { return *(this->value) * n; }
	};

	auto size = std::size_t(10);
	auto data = arange<TestType>(size);
	auto view = scip::View<TestType, Proxy>(data.get(), size);

	SECTION("Views can be iterated using range based for loop") {
		auto sumx2 = TestType{0};
		for (auto v : view)
			sumx2 += v.times(2);
		REQUIRE(sumx2 == static_cast<decltype(sumx2)>(size * (size - 1)));
	}

	SECTION("Views work with the standard library") {
		auto times2 = std::list<TestType>{};
		std::transform(view.begin(), view.end(), std::back_inserter(times2), [](auto proxy) {
			return proxy.times(2);
		});
		auto sumx2 = std::accumulate(times2.begin(), times2.end(), TestType{0});
		REQUIRE(sumx2 == static_cast<decltype(sumx2)>(size * (size - 1)));
	}
}
