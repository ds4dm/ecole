#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class FileGenerator : public InstanceGenerator {
public:
	struct Parameters {
		enum struct SamplingMode { replace, remove, remove_and_repeat };

		bool recursive = true;
		SamplingMode sampling_mode = SamplingMode::remove_and_repeat;
	};

	FileGenerator(std::filesystem::path const& dir, Parameters parameters, RandomEngine random_engine);
	FileGenerator(std::filesystem::path const& dir, Parameters parameters);
	FileGenerator(std::filesystem::path const& dir);

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
