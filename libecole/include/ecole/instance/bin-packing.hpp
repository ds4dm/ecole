#ifndef BIN_PACKING_HPP
#define BIN_PACKING_HPP
#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "ecole/export.hpp"
#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class ECOLE_EXPORT Binpacking : public InstanceGenerator {
public:
	struct ECOLE_EXPORT Parameters {
		std::string filename;  // NOLINT(readability-magic-numbers)
		std::size_t n_bins;    // NOLINT(readability-magic-numbers)
	};

	ECOLE_EXPORT static scip::Model generate_instance(Parameters parameters, RandomGenerator& rng);

	ECOLE_EXPORT Binpacking(Parameters parameters, RandomGenerator rng);
	ECOLE_EXPORT Binpacking(Parameters parameters);
	ECOLE_EXPORT Binpacking();

	ECOLE_EXPORT scip::Model next() override;
	ECOLE_EXPORT void seed(Seed seed) override;
	[[nodiscard]] ECOLE_EXPORT bool done() const override { return false; }

	[[nodiscard]] ECOLE_EXPORT Parameters const& get_parameters() const noexcept { return parameters; }

private:
	RandomGenerator rng;
	Parameters parameters;
};

}  // namespace ecole::instance

#endif