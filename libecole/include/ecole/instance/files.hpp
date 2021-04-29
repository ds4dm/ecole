#pragma once

#include <filesystem>
#include <vector>

#include "ecole/instance/abstract.hpp"
#include "ecole/random.hpp"

namespace ecole::instance {

class FilesGenerator : public InstanceGenerator {
public:
	struct Parameters {
		bool recursive = true;
	};

	FilesGenerator(std::filesystem::path const& dir, Parameters parameters, RandomEngine random_engine);
	FilesGenerator(std::filesystem::path const& dir, Parameters parameters);
	FilesGenerator(std::filesystem::path const& dir);

	scip::Model next() override;
	void seed(Seed seed) override;

private:
	RandomEngine random_engine;
	Parameters parameters;
	std::vector<std::filesystem::path> files;
};

}  // namespace ecole::instance
