#pragma once

#include <memory>

#include "ecole/base/environment.hpp"
#include "ecole/observation.hpp"

namespace ecole {
namespace py {

struct ObsBase {
	virtual ~ObsBase() = default;
};

using ObsSpaceBase = base::ObservationSpace<std::unique_ptr<ObsBase>>;

template <typename T_Obs> struct Obs : public ObsBase {
	T_Obs obs;
	Obs(T_Obs&& obs) : obs(std::move(obs)) {}
};

template <typename T_ObsSpace> struct ObsSpace : ObsSpaceBase {
	using embd_obs_t = typename T_ObsSpace::obs_t;
	using py_obs_t = Obs<embd_obs_t>;
	using obs_t = std::unique_ptr<ObsBase>;

	T_ObsSpace obs_space;

	ObsSpace(T_ObsSpace const& obs_space) : obs_space(obs_space) {}
	ObsSpace(T_ObsSpace&& obs_space) : obs_space(std::move(obs_space)) {}
	template <typename... Args>
	ObsSpace(Args... args) : obs_space(std::forward<Args>(args)...) {}

	obs_t get(scip::Model const& model) const override {
		return std::make_unique<py_obs_t>(obs_space.get(model));
	}
	std::unique_ptr<ObsSpaceBase> clone() const override {
		return std::make_unique<ObsSpace>(obs_space);
	}
};

using BasicObs = py::Obs<obs::BasicObs>;
using BasicObsSpace = py::ObsSpace<obs::BasicObsSpace>;

} // namespace py
} // namespace ecole
