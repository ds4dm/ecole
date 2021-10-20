#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "ecole/export.hpp"
#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class ECOLE_EXPORT FileGenerator : public InstanceGenerator {
public:
	struct ECOLE_EXPORT Parameters {
		enum struct ECOLE_EXPORT SamplingMode { replace, remove, remove_and_repeat };

		// FIXME Made this a string for easier bining while waiting for PyBind 2.7
		// https://github.com/pybind/pybind11/pull/2730
		std::string directory = "instances";
		bool recursive = true;
		SamplingMode sampling_mode = SamplingMode::remove_and_repeat;
	};

	ECOLE_EXPORT FileGenerator(Parameters parameters, RandomEngine random_engine);
	ECOLE_EXPORT FileGenerator(Parameters parameters);
	ECOLE_EXPORT FileGenerator();

	ECOLE_EXPORT auto next() -> scip::Model override;
	ECOLE_EXPORT void seed(Seed seed) override;
	[[nodiscard]] ECOLE_EXPORT auto done() const -> bool override;

	[[nodiscard]] ECOLE_EXPORT auto get_parameters() const noexcept -> Parameters const& { return parameters; }

private:
	RandomEngine random_engine;
	Parameters parameters;
	std::vector<std::filesystem::path> files;
	std::size_t files_remaining;

	void reset_file_list();
};

}  // namespace ecole::instance
