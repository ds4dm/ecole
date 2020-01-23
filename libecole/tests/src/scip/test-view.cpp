#include <cstddef>
#include <iterator>
#include <list>
#include <memory>

#include <catch2/catch.hpp>

#include "ecole/exception.hpp"
#include "ecole/scip/view.hpp"

using namespace ecole;

template <typename T> auto arange(std::size_t size) {
	auto deleter = [size](T* const* data) {
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
	return std::unique_ptr<T* const, decltype(deleter)>(make_data(), deleter);
}

TEMPLATE_TEST_CASE("View represent a pointer on data", "", int, double) {
	struct Proxy : public scip::Proxy<TestType> {
		Proxy(Scip*, TestType* value) noexcept : scip::Proxy<TestType>(nullptr, value) {}
		TestType times(TestType n) const noexcept { return *(this->value) * n; }
	};

	auto size = std::size_t(10);
	auto data = arange<TestType>(size);
	auto view = scip::View<Proxy>(nullptr, data.get(), size);

	SECTION("Proxies can be compared") { REQUIRE(view[0] == view[0]); }

	SECTION("Can be iterated using range based for loop") {
		auto sumx2 = TestType{0};
		for (auto v : view)
			sumx2 += v.times(2);
		REQUIRE(sumx2 == static_cast<decltype(sumx2)>(size * (size - 1)));
	}

	SECTION("Random access iterator") {
		auto iter = view.begin();
		REQUIRE(iter[size - 1].times(1) == static_cast<TestType>(size - 1));
	}

	SECTION("View accessor") {
		REQUIRE(view[size - 1].times(1) == static_cast<TestType>(size - 1));
		REQUIRE_THROWS_AS(view.at(size), scip::Exception);
	}

	SECTION("Work with the standard library") {
		auto times2 = std::list<TestType>{};
		std::transform(view.begin(), view.end(), std::back_inserter(times2), [](auto proxy) {
			return proxy.times(2);
		});
		auto sumx2 = std::accumulate(times2.begin(), times2.end(), TestType{0});
		REQUIRE(sumx2 == static_cast<decltype(sumx2)>(size * (size - 1)));
	}
}
