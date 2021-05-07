#include <algorithm>
#include <array>

#include <catch2/catch.hpp>

#include "ecole/exception.hpp"
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
	InstanceDatasetRAII(bool nested = false) {
		auto model = get_model();
		for (auto const* const name : names) {
			model.set_name(name);
			if (nested) {
				auto dir = this->make_subpath();
				std::filesystem::create_directory(dir);
				model.write_problem(dir / "model.mps");
			} else {
				model.write_problem(this->make_subpath(".mps"));
			}
		}
	}
};

/** Check if one container is a sumbet of the other. */
template <typename RangeA, typename RangeB> auto is_subset(RangeA const& range_a, RangeB const& range_b) -> bool {
	// Naive quadratic algorithm
	auto in_range_b = [&range_b](auto elem) { return std::find(begin(range_b), end(range_b), elem) < end(range_b); };
	return std::all_of(range_a.begin(), range_a.end(), in_range_b);
}

/** Check if two containers represent the same set. */
template <typename RangeA, typename RangeB> auto is_same_set(RangeA const& range_a, RangeB const& range_b) -> bool {
	// Naive algorithm
	return is_subset(range_a, range_b) && is_subset(range_b, range_a);
}

/** Collect model name from a generator. */
template <std::size_t N> auto collect_names(instance::FileGenerator& generator) {
	auto names = std::array<std::string, N>{};
	std::generate(begin(names), end(names), [&]() { return generator.next().name(); });
	return names;
}

TEST_CASE("FileGenerator properly iterate over files", "[instance]") {
	using SamplingMode = instance::FileGenerator::Parameters::SamplingMode;
	auto const nested_dirs = GENERATE(true, false);
	auto const recursive = GENERATE(true, false);
	auto const sampling_mode = GENERATE(SamplingMode::replace, SamplingMode::remove, SamplingMode::remove_and_repeat);
	auto const instances_raii = InstanceDatasetRAII{nested_dirs};
	auto generator = instance::FileGenerator{{instances_raii.dir(), recursive, sampling_mode}};

	if (nested_dirs && !recursive) {
		SECTION("Throw exception when no files are found") {
			REQUIRE(generator.done());
			REQUIRE_THROWS_AS(generator.next(), IteratorExhausted);
		}

	} else {
		SECTION("Iterate as many files as there are in the dateset") {
			auto constexpr n_files = InstanceDatasetRAII::names.size();
			auto names_seen = collect_names<n_files>(generator);

			if (sampling_mode == SamplingMode::remove) {
				SECTION("Throw exception after first iteration when removing.") {
					REQUIRE(generator.done());
					REQUIRE_THROWS_AS(generator.next(), IteratorExhausted);
				}
			} else {
				SECTION("Iterate over files a second time when replacing or repeating ") { collect_names<n_files>(generator); }
			}
			if (sampling_mode == SamplingMode::replace) {
				REQUIRE(is_subset(names_seen, InstanceDatasetRAII::names));
			} else {
				REQUIRE(is_same_set(names_seen, InstanceDatasetRAII::names));
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
}
