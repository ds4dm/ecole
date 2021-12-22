#pragma once

#include <utility>

namespace ecole::scip {
class Model;
}

namespace ecole::data {

template <typename Data> class ConstantFunction {
public:
	ConstantFunction() = default;
	ConstantFunction(Data data_) : data{std::move(data_)} {}

	auto before_reset(scip::Model const& /*model*/) -> void {}

	[[nodiscard]] auto extract(scip::Model const& /* model */, bool /* done */) const -> Data { return data; };

private:
	Data data;
};

}  // namespace ecole::data
