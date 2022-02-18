#include <catch2/catch.hpp>
#include <memory>
#include <variant>

#include "ecole/none.hpp"
#include "ecole/scip/scimpl.hpp"
#include "ecole/utility/coroutine.hpp"

using namespace ecole;

TEST_CASE("Coroutine manage ressources", "[utility]") {
	using Coroutine = utility::Coroutine<NoneType, NoneType>;
	using Executor = Coroutine::Executor;

	SECTION("Coroutine teminate immediatly") {
		auto co = Coroutine{[](Executor const& /*executor*/) {}};

		SECTION("Being waited on") {
			auto ret = co.wait();
			REQUIRE_FALSE(ret.has_value());
		}
	}

	SECTION("Coroutine is killed in execution") {
		auto co = Coroutine{[](Executor& executor) {
			auto message = executor.yield(None);
			do {
				if (Executor::is_stop(message)) {
					break;
				}
				message = executor.yield(None);
			} while (true);
		}};

		SECTION("Being waited on") {
			auto ret = co.wait();
			REQUIRE(ret.has_value());
		}
	}
}

TEST_CASE("Coroutine can return values", "[utility]") {
	using Coroutine = utility::Coroutine<int, NoneType>;
	using Executor = Coroutine::Executor;
	auto const max = GENERATE(0, 1, 5);

	auto co = Coroutine{[max](Executor& executor) {
		for (int i = 0; i < max; ++i) {
			auto message = executor.yield(i);
			if (Executor::is_stop(message)) {
				break;
			}
		}
	}};

	for (int i = 0; i < max; ++i) {
		auto ret = co.wait();
		REQUIRE(ret.has_value());
		REQUIRE(ret.value() == i);
		co.resume(None);
	}
	REQUIRE_FALSE(co.wait().has_value());
}

TEST_CASE("Coroutine can send messages", "[utility]") {
	using Coroutine = utility::Coroutine<int, int>;
	using Executor = Coroutine::Executor;
	auto const max = GENERATE(0, 1, 5);

	auto co = Coroutine{[max](Executor& executor) {
		int last_message = 0;
		while (true) {
			auto message = executor.yield(last_message);
			if (Executor::is_stop(message)) {
				break;
			}
			last_message = std::get<int>(message);
		}
	}};

	auto ret = co.wait();
	REQUIRE(ret.has_value());
	constexpr auto message = 10;
	co.resume(message);
	ret = co.wait();
	REQUIRE(ret.has_value());
	REQUIRE(ret.value() == message);
}
