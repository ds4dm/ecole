#include <algorithm>
#include <cstddef>
#include <random>

#include "test-utility/tmp-folder.hpp"

namespace ecole {

namespace fs = std::filesystem;

namespace {

/** Return a random alphanumeric string with `n` charaters. */
auto random_alphanumeric(std::size_t n) -> std::string {
	auto constexpr chars = std::string_view{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};
	auto choice = std::uniform_int_distribution<std::size_t>{0, chars.length() - 1};
	auto rng = std::minstd_rand{std::random_device{}()};
	auto out = std::string(n, '0');
	std::generate(begin(out), end(out), [&]() { return chars[choice(rng)]; });
	return out;
}

}  // namespace

TmpFolderRAII::TmpFolderRAII() {
	tmp_dir = fs::temp_directory_path() / ("ecole-test-" + random_alphanumeric(rand_size));
	fs::create_directories(tmp_dir);
}

TmpFolderRAII::~TmpFolderRAII() {
	fs::remove_all(tmp_dir);
}

auto TmpFolderRAII::make_subpath(std::string_view suffix) const -> std::filesystem::path {
	return tmp_dir / random_alphanumeric(rand_size).append(suffix);
}

}  // namespace ecole
