#include <string>
#include <tuple>
#include <utility>

#include <fmt/format.h>
#include <fmt/ranges.h>

namespace ecole::benchmark {

template <typename... Args> auto make_csv(Args&&... args) -> std::string {
	return fmt::format(R"("{}")", fmt::join(std::tuple{std::forward<Args>(args)...}, R"(",")"));
}

template <typename... Args> auto merge_csv(Args&&... args) -> std::string {
	return fmt::format("{}", fmt::join(std::tuple{std::forward<Args>(args)...}, ","));
}

}  // namespace ecole::benchmark
