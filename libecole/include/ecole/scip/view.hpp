#pragma once

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>

namespace ecole {
namespace scip {

template <typename T> class Proxy {
protected:
	T* const value;

public:
	Proxy() = delete;
	explicit Proxy(T* value) noexcept : value(value) {}
	virtual ~Proxy() {}
};

template <typename T, typename Proxy> class View {
public:
	std::size_t const size;

private:
	T* const* const data;

	class ViewIterator {
	private:
		T* const* ptr;

	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = Proxy;
		using difference_type = std::ptrdiff_t;
		using pointer = void;
		using reference = void;

		ViewIterator() = delete;
		explicit ViewIterator(T* const* ptr) noexcept : ptr(ptr) {}

		inline bool operator==(ViewIterator other) const { return ptr == other.ptr; }
		inline bool operator!=(ViewIterator other) const { return !(*this == other); }
		inline friend bool operator<(ViewIterator lhs, ViewIterator rhs) {
			return lhs.ptr < rhs.ptr;
		}
		inline friend bool operator>(ViewIterator lhs, ViewIterator rhs) { return rhs < lhs; }
		inline friend bool operator<=(ViewIterator lhs, ViewIterator rhs) {
			return !(lhs > rhs);
		}
		inline friend bool operator>=(ViewIterator lhs, ViewIterator rhs) {
			return !(lhs < rhs);
		}

		inline ViewIterator& operator++() {
			ptr++;
			return *this;
		}
		inline ViewIterator operator++(int) {
			auto retval = *this;
			++(*this);
			return retval;
		}
		inline ViewIterator& operator+=(difference_type n) {
			ptr += n;
			return *this;
		}
		inline ViewIterator& operator-=(difference_type n) { return *this += (-n); }
		inline friend ViewIterator operator+(ViewIterator iter, difference_type n) {
			return iter += n;
		}
		inline friend ViewIterator operator-(ViewIterator iter, difference_type n) {
			return iter -= n;
		}
		inline friend difference_type operator-(ViewIterator a, ViewIterator b) {
			return a.ptr - b.ptr;
		}

		inline value_type operator*() const { return value_type{*ptr}; }
		inline value_type operator[](difference_type n) const { return *(*this + n); }
	};

public:
	View() = delete;
	explicit View(T* const* data, std::size_t size) noexcept : size(size), data(data) {}

	inline auto cbegin() const { return ViewIterator(data); }
	inline auto begin() const { return cbegin(); }
	inline auto cend() const { return ViewIterator(data + size); }
	inline auto end() const { return cend(); }

	inline auto operator[](std::size_t n) const {
		return cbegin()[static_cast<std::ptrdiff_t>(n)];
	}
	auto at(std::size_t n) const {
		if (n < 0 || n >= size)
			throw std::out_of_range("Out of range: " + std::to_string(n));
		else
			return (*this)[n];
	}
};

} // namespace scip
} // namespace ecole
