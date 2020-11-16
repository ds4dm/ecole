#pragma once

#include <utility>

#include "ecole/data/abstract.hpp"

namespace ecole::data {

template <typename Data> class ConstantFunction : public DataFunction<Data> {
public:
	ConstantFunction() = default;
	ConstantFunction(Data data_) : data{std::move(data_)} {}

	Data extract(scip::Model& /* model */, bool /* done */) override { return data; };

private:
	Data data;
};

}  // namespace ecole::data
