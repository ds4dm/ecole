#pragma once

#include <memory>
#include <string>
#include <tuple>

#include "ecole/exception.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace base {

template <typename Observation> struct ObservationSpace {
	using obs_t = Observation;

	virtual ~ObservationSpace() = default;
	virtual obs_t get(scip::Model const& model) = 0;
	virtual std::unique_ptr<ObservationSpace> clone() const = 0;
};

struct RewardSpace {
	using reward_t = double;

	virtual ~RewardSpace() = default;
	virtual void reset(scip::Model const& model);
	virtual reward_t step(scip::Model const& model);
	virtual std::unique_ptr<RewardSpace> clone() const = 0;
};

template <typename Observation, typename Action> class Env {
public:
	using action_t = Action;
	using obs_t = Observation;
	using reward_t = RewardSpace::reward_t;
	using seed_t = int;
	using info_t = int;

	virtual ~Env() = default;

	seed_t seed(seed_t seed) noexcept;
	seed_t seed() const noexcept;
	std::tuple<obs_t, bool> reset(scip::Model model);
	std::tuple<obs_t, bool> reset(std::string filename);
	std::tuple<obs_t, reward_t, bool, info_t> step(action_t action);

private:
	bool can_transition = false;
	seed_t _seed = 0;

	void mutate_seed() noexcept;
	virtual std::tuple<obs_t, bool> _reset(scip::Model model) = 0;
	virtual std::tuple<obs_t, reward_t, bool, info_t> _step(action_t action) = 0;
};

template <typename O, typename A> auto Env<O, A>::seed(seed_t seed) noexcept -> seed_t {
	return _seed = seed;
}

template <typename O, typename A> auto Env<O, A>::seed() const noexcept -> seed_t {
	return _seed;
}

template <typename O, typename A>
auto Env<O, A>::reset(scip::Model model) -> std::tuple<obs_t, bool> {
	mutate_seed();
	try {
		auto result = _reset(std::move(model));
		can_transition = !std::get<1>(result);
		return result;
	} catch (std::exception const&) {
		can_transition = false;
		throw;
	}
}

template <typename O, typename A>
auto Env<O, A>::reset(std::string filename) -> std::tuple<obs_t, bool> {
	return reset(scip::Model::from_file(filename));
}

template <typename O, typename A>
auto Env<O, A>::step(action_t action) -> std::tuple<obs_t, reward_t, bool, info_t> {
	if (!can_transition) throw Exception("Environment need to be reset.");
	try {
		auto result = _step(std::move(action));
		can_transition = !std::get<2>(result);
		return result;
	} catch (std::exception const&) {
		can_transition = false;
		throw;
	}
}

template <typename O, typename A> void Env<O, A>::mutate_seed() noexcept {
	++_seed;
}

}  // namespace base
}  // namespace ecole
