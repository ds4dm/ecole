#include <algorithm>
#include <array>

#include <catch2/catch.hpp>

#include "ecole/instance/files.hpp"

#include "conftest.hpp"
#include "test-utility/tmp-folder.hpp"

using namespace ecole;

/** Ressource management class to create a local dataset. */
class InstanceDatasetRAII : public TmpFolderRAII {
public:
	/** Number of files to create. */
	inline static constexpr auto names = std::array{"m1", "m2", "m3", "m4"};

	/** Generate a local dataset of instances. */
	InstanceDatasetRAII() {
		auto model = get_model();
		for (auto const* const name : names) {
			model.set_name(name);
			model.write_problem(this->make_subpath(".mps"));
		}
	}
};

/** Check wether an element is inside a container. */
template <typename Elem, typename Range> auto is_in(Elem const& val, Range const& range) -> bool {
	return std::find(begin(range), end(range), val) != end(range);
}

TEST_CASE("FilesGenerator unit test", "[unit][instance]") {
	// We cannot run the usual instance generator unit tests because the loader is not as complete.
	auto const instances_raii = InstanceDatasetRAII{};
	auto generator = instance::FilesGenerator{instances_raii.dir()};

	SECTION("Generate instances in a loop") {
		auto const n_instances = GENERATE(2, 2 * InstanceDatasetRAII::names.size());
		for (auto i = 0; i < n_instances; ++i) {
			auto model = generator.next();
		}
	}

	SECTION("Same seed give reproducible results") {
		generator.seed(0);
		auto const model1 = generator.next();
		generator.seed(0);
		auto const model2 = generator.next();
		REQUIRE(model1.name() == model2.name());
	}
}

TEST_CASE("FilesGenerator properly iterate over files", "[instance]") {
	auto const instances_raii = InstanceDatasetRAII{};
	auto generator = instance::FilesGenerator{instances_raii.dir()};

	SECTION("Files are from the dataset") { REQUIRE(is_in(generator.next().name(), InstanceDatasetRAII::names)); }
}
