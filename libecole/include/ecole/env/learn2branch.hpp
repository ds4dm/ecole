#pragma once

#include <memory>

#include <string>

#include "ecole/env/observation.hpp"
#include "ecole/scip/model.hpp"

namespace ecole {
namespace env {

class BranchEnv {
protected:
	scip::Model model;
	std::unique_ptr<Observation::Factory> factory;

public:
	using BranchFunc = std::function<std::size_t(std::unique_ptr<Observation>)>;
	using ObsFactory = Observation::Factory;

	BranchEnv(scip::Model&& model, std::unique_ptr<ObsFactory> factory) noexcept;

	void run(BranchFunc const& func);
};

} // namespace env
} // namespace ecole
