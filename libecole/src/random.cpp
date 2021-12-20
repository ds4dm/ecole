#include <locale>
#include <mutex>
#include <sstream>

#include "ecole/random.hpp"

namespace ecole {
namespace {

class RandomGeneratorManager {
public:
	static auto get() -> RandomGeneratorManager&;

	auto seed(Seed val) -> void;
	auto spawn() -> RandomGenerator;

private:
	std::mutex m;
	Seed user_seed = 0;
	Seed spawn_seed = 0;

	RandomGeneratorManager();

	auto new_seed_seq() -> std::seed_seq;
};

}  // namespace

auto seed(Seed val) -> void {
	RandomGeneratorManager::get().seed(val);
}

auto spawn_random_generator() -> RandomGenerator {
	return RandomGeneratorManager::get().spawn();
}

// Not efficient, but operator<< is the only thing we have
auto serialize(RandomGenerator const& rng) -> std::string {
	auto osstream = std::ostringstream{};
	osstream.imbue(std::locale("C"));
	osstream << rng;
	return std::move(osstream).str();
}

// Not efficient, but operator>> is the only thing we have
auto deserialize(std::string const& data) -> RandomGenerator {
	auto rng = RandomGenerator{};  // NOLINT need not be seeded since we set its state
	auto isstream = std::istringstream{data};
	isstream.imbue(std::locale("C"));
	std::move(isstream) >> rng;
	return rng;
}

/*******************************************
 *  Implementation of RandomGeneratorManager  *
 *******************************************/

namespace {

auto RandomGeneratorManager::get() -> RandomGeneratorManager& {
	static auto rng = RandomGeneratorManager{};
	return rng;
}

auto RandomGeneratorManager::seed(Seed val) -> void {
	auto const lk = std::unique_lock{m};
	user_seed = val;
	spawn_seed = 0;
}

auto RandomGeneratorManager::spawn() -> RandomGenerator {
	auto seeds = new_seed_seq();
	return RandomGenerator{seeds};
}

RandomGeneratorManager::RandomGeneratorManager() : user_seed{std::random_device{}()} {}

auto RandomGeneratorManager::new_seed_seq() -> std::seed_seq {
	auto const lk = std::unique_lock{m};
	return {user_seed, ++spawn_seed};
}

}  // namespace
}  // namespace ecole
