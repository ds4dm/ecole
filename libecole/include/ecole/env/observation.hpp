#pragma once

#include <memory>

#include "ecole/scip/model.hpp"

namespace ecole {
namespace env {

class Observation {
public:
	class Factory {
	public:
		virtual std::unique_ptr<Observation> make(scip::Model const& model) = 0;
		virtual ~Factory() = default;
	};

	virtual ~Observation() = default;
};

class BasicObs : public Observation {
public:
	class Factory : public Observation::Factory {
	public:
		std::unique_ptr<Observation> make(scip::Model const& model) override;
		~Factory() = default;
	};

	~BasicObs() = default;
};

} // namespace env
} // namespace ecole
