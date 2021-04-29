#pragma once

#include <filesystem>
#include <string_view>

namespace ecole {

/** A ressource management to create and clean temporary files, */
class TmpFolderRAII {
public:
	/** Size of the random string used to create unique names. */
	inline static auto constexpr rand_size = 10;

	/** Create an empty temporary directory. */
	TmpFolderRAII();
	/** Delete the temporary directory and all its content. */
	~TmpFolderRAII();

	TmpFolderRAII(TmpFolderRAII const&) = delete;
	auto operator=(TmpFolderRAII const&) -> TmpFolderRAII& = delete;

	/* Get the name of a new unique sub path inside the tmeporary directory. **/
	[[nodiscard]] auto make_subpath(std::string_view suffix = "") const -> std::filesystem::path;

	[[nodiscard]] auto dir() const -> std::filesystem::path const& { return tmp_dir; }

private:
	std::filesystem::path tmp_dir;
};

}  // namespace ecole
