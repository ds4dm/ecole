#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class FileGenerator : public InstanceGenerator {
public:
	struct Parameters {
		enum struct SamplingMode { replace, remove, remove_and_repeat };

		// FIXME Made this a string for easier bining while waiting for PyBind 2.7
		// https://github.com/pybind/pybind11/pull/2730
		std::string directory = "instances";
		bool recursive = true;
		SamplingMode sampling_mode = SamplingMode::remove_and_repeat;
	};

	FileGenerator(Parameters parameters, RandomEngine random_engine);
	FileGenerator(Parameters parameters);
	FileGenerator();

	auto next() -> scip::Model override;
	void seed(Seed seed) override;
	[[nodiscard]] auto done() const -> bool override;

	[[nodiscard]] auto get_parameters() const noexcept -> Parameters const& { return parameters; }

private:
	RandomEngine random_engine;
	Parameters parameters;
	std::vector<std::filesystem::path> files;
	std::size_t files_remaining;

	void reset_file_list();
};

}  // namespace ecole::instance
