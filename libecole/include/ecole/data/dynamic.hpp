#pragma once

#include <memory>
#include <utility>

#include "ecole/data/abstract.hpp"

namespace ecole::data {

/**
 * Type erased wrapper for data function with similar data.
 *
 * This class allow to wrap any type of data function with similar data inside a wrapper of the same type.
 * This enables dynamic polymorphism for data functions.
 * For instance, using ``DynamicFunction<Reward>``, one can store any other reward function inside a
 * container (``std::vector``, ``std::map``...).
 *
 * @tparam Data Type of data returned by this function. All wrapped functions must be able to extracc
 *         data convertible to this type.
 *         This can be achieved for instance by choosing ``Data`` to be ``std::variant``.
 */
template <typename Data> class DynamicFunction {
public:
	/** Create a ``DynamicFunction`` from any compatible data function. */
	template <typename DataFunction>
	explicit DynamicFunction(DataFunction data_function) :
		m_pimpl{std::make_unique<DataFunctionWrapper<DataFunction>>(std::move(data_function))} {}

	/** Default move semantics. */
	DynamicFunction(DynamicFunction&&) noexcept = default;
	/** Copy by copying the wrapped data function. */
	DynamicFunction(DynamicFunction const& other) : DynamicFunction{other.m_pimpl->clone()} {}

	/** Default move assign semantics. */
	DynamicFunction& operator=(DynamicFunction&&) noexcept = default;
	/** Copy assign by copying the wrapped data function. */
	DynamicFunction& operator=(DynamicFunction const& other) {
		if (this != &other) {
			m_pimpl = other.m_pimpl->clone();
		}
		return *this;
	}

	/** Call ``before_reset`` onto the wrapped item. */
	auto before_reset(scip::Model& model) -> void { return m_pimpl->before_reset(model); }

	/** Call ``extract`` onto the wrapped item. */
	auto extract(scip::Model& model, bool done) -> Data { return m_pimpl->extract(model, done); }

private:
	/**
	 * Interface expected of a data function.
	 *
	 * This is used for dynamic polymorphism based on virtual inheritance.
	 */
	struct DataFunctionAbstract {
		virtual ~DataFunctionAbstract() = default;
		virtual auto clone() -> std::unique_ptr<DataFunctionAbstract> = 0;
		virtual auto before_reset(scip::Model& model) -> void = 0;
		virtual auto extract(scip::Model& model, bool done) -> Data = 0;
	};

	/**
	 * Wrapper for any compatible data function.
	 *
	 * The wrapper implements the ``DataFunctionAbstract`` interface.
	 */
	template <typename DataFunction> struct DataFunctionWrapper final : DataFunctionAbstract {
		explicit DataFunctionWrapper(DataFunction data_function) : m_data_function{std::move(data_function)} {}
		auto clone() -> std::unique_ptr<DataFunctionAbstract> override {
			return std::make_unique<DataFunctionWrapper>(*this);
		}
		auto before_reset(scip::Model& model) -> void override { return m_data_function.before_reset(model); }
		auto extract(scip::Model& model, bool done) -> Data override { return m_data_function.extract(model, done); }

		DataFunction m_data_function;
	};

	explicit DynamicFunction(std::unique_ptr<DataFunctionAbstract> ptr) : m_pimpl{std::move(ptr)} {};

	std::unique_ptr<DataFunctionAbstract> m_pimpl;
};

}  // namespace ecole::data
