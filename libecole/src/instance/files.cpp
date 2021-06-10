#include <algorithm>
#include <iterator>
#include <random>

#include "ecole/exception.hpp"
#include "ecole/instance/files.hpp"

namespace ecole::instance {

namespace fs = std::filesystem;

namespace {

/** List files and symlinks to files in the given directory iterator. */
template <typename FileIter> auto list_files(FileIter&& dir_iter) {
	auto files = std::vector<fs::path>{};
	for (auto iter = begin(dir_iter), last = end(dir_iter); iter != last; ++iter) {
		auto file = iter->path();
		if (fs::is_regular_file(file) || (fs::is_symlink(file) && fs::exists(fs::read_symlink(file)))) {
			files.push_back(std::move(file));
		}
	}
	return files;
}

}  // namespace

FileGenerator::FileGenerator(Parameters parameters_, RandomEngine random_engine_) :
	random_engine{random_engine_}, parameters{std::move(parameters_)} {
	using opts = fs::directory_options;
	if (parameters.recursive) {
		files = list_files(fs::recursive_directory_iterator{parameters.directory, opts::follow_directory_symlink});
	} else {
		files = list_files(fs::directory_iterator{parameters.directory, opts::follow_directory_symlink});
	}
	// The order in which the files are iterated over is unspecified.
	reset_file_list();
}

FileGenerator::FileGenerator(Parameters parameters_) :
	FileGenerator{std::move(parameters_), ecole::spawn_random_engine()} {}

FileGenerator::FileGenerator() : FileGenerator{Parameters{}} {}

auto FileGenerator::next() -> scip::Model {
	if (done()) {
		throw IteratorExhausted{};
	}
	if (files_remaining == 0) {
		files_remaining = files.size();
	}

	auto choice = std::uniform_int_distribution<std::size_t>{0, files_remaining - 1};
	auto const idx = choice(random_engine);

	// files_remaining is not used in this case, it is only an alias for files.size().
	if (parameters.sampling_mode == Parameters::SamplingMode::replace) {
		return scip::Model::from_file(files[idx]);
	}

	// files[0: files_reamining] are unseen files, while files[files_reamining: -1] are seen.
	// We mark files[idx] as seen by exchanging it with files[files_remaining]
	files_remaining--;
	swap(files[idx], files[files_remaining]);
	return scip::Model::from_file(files[files_remaining]);
}

void FileGenerator::seed(Seed seed) {
	reset_file_list();
	random_engine.seed(seed);
}

auto FileGenerator::done() const -> bool {
	auto const no_files_at_all = files.empty();
	auto const seen_all_files = (files_remaining == 0 && parameters.sampling_mode == Parameters::SamplingMode::remove);
	return no_files_at_all || seen_all_files;
}

void FileGenerator::reset_file_list() {
	std::sort(begin(files), end(files));
	files_remaining = files.size();
}

}  // namespace ecole::instance
