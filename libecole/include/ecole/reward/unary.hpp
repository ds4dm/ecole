#pragma once

#include <utility>

#include "ecole/reward/abstract.hpp"

namespace ecole::reward {

/** A class to map unary opertaion (such as exp(), log()...) onto rewards. */
template <typename RewardFunc, typename UnaryFunc> class UnaryFunction : RewardFunction {
public:
	/** Default construct all functions. */
	UnaryFunction() = default;

	/** Store copies of the functions. */
	UnaryFunction(RewardFunc reward_func, UnaryFunc unary_func) :
		reward_function{std::move(reward_func)}, unary_function{std::move(unary_func)} {}

	/** Call reset on the reward function. */
	void reset(scip::Model& model) override { reward_function.reset(model); }

	/** Extract reward and call the unary operation on it. */
	Reward extract(scip::Model& model, bool done = false) override {
		return unary_function(reward_function.extract(model, done));
	}

private:
	RewardFunc reward_function;
	UnaryFunc unary_function;
};

}  // namespace ecole::reward
