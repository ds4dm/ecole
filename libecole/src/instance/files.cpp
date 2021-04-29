#include <algorithm>
#include <iterator>
#include <random>
#include <stdexcept>

#include "ecole/instance/files.hpp"

namespace ecole::instance {

namespace fs = std::filesystem;

namespace {

template <typename FileIter> auto list_files(FileIter&& iter) {
	auto files = std::vector<fs::path>{};
	std::transform(begin(iter), end(iter), std::back_inserter(files), [](auto de) { return de.path(); });
	// The order in which the files are iterated over is unspecified.
	std::sort(begin(files), end(files));
	return files;
}

}  // namespace

FilesGenerator::FilesGenerator(fs::path const& dir, Parameters parameters_, RandomEngine random_engine_) :
	random_engine{random_engine_}, parameters{parameters_} {
	if (parameters.recursive) {
		files = list_files(fs::recursive_directory_iterator{dir});
	} else {
		files = list_files(fs::directory_iterator{dir});
	}
}

FilesGenerator::FilesGenerator(std::filesystem::path const& dir, Parameters parameters_) :
	FilesGenerator{dir, parameters_, ecole::spawn_random_engine()} {}

FilesGenerator::FilesGenerator(std::filesystem::path const& dir) :
	FilesGenerator{dir, Parameters{}, ecole::spawn_random_engine()} {}

void FilesGenerator::seed(Seed seed) {
	random_engine.seed(seed);
}

scip::Model FilesGenerator::next() {
	if (files.empty()) {
		throw std::out_of_range{"No files to iterate over."};
	}
	auto choice = std::uniform_int_distribution<std::size_t>{0, files.size() - 1};
	return scip::Model::from_file(files[choice(random_engine)]);
}

}  // namespace ecole::instance
