#pragma once

#include <cstddef>
#include <iterator>

namespace ecole {
namespace scip {

template <typename T> class Proxy {
protected:
	T const* const value;

public:
	Proxy() = delete;
	explicit Proxy(T const* value) noexcept : value(value) {}
	virtual ~Proxy() {}
};

template <typename T, typename Proxy> class View {
private:
	T const* const* const data;
	std::size_t const size;

	class ViewIterator {
	private:
		T const* const* ptr;

	public:
		using iterator_category = std::input_iterator_tag;
		using value_type = Proxy;
		using difference_type = void;
		using pointer = void;
		using reference = void;

		ViewIterator() = delete;
		explicit ViewIterator(T const* const* ptr) noexcept : ptr(ptr) {}
		ViewIterator& operator++() {
			ptr++;
			return *this;
		}
		ViewIterator operator++(int) {
			auto retval = *this;
			++(*this);
			return retval;
		}
		bool operator==(ViewIterator other) const { return ptr == other.ptr; }
		bool operator!=(ViewIterator other) const { return !(*this == other); }
		value_type operator*() const { return *ptr; }
	};

public:
	View() = delete;
	explicit View(T const* const* data, std::size_t size) noexcept :
			data(data),
			size(size) {}

	auto cbegin() const { return ViewIterator(data); }
	auto begin() const { return cbegin(); }
	auto cend() const { return ViewIterator(data + size); }
	auto end() const { return cend(); }
};

} // namespace scip
} // namespace ecole
