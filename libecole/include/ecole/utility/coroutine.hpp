#pragma once

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <variant>

namespace ecole::utility {

template <typename Yield, typename Message> class Coroutine {
public:
	template <class Function, class... Args> Coroutine(Function&& func, Args&&... args);
	~Coroutine() noexcept;

	auto wait_executor() -> void;
	auto resume_executor(Message message) -> void;
	[[nodiscard]] auto is_done() const noexcept -> bool;

private:
	struct StopToken {};

	using MessageOrStop = std::variant<Message, Coroutine::StopToken>;
	using Lock = std::unique_lock<std::mutex>;

	class Synchronizer {
	public:
		auto coroutine_wait_executor() -> Lock;
		auto coroutine_resume_executor(Lock&& lk, MessageOrStop message) -> void;
		auto coroutine_stop_executor(Lock&& lk) -> void;
		[[nodiscard]] auto coroutine_executor_is_done(Lock const& lk) const noexcept -> bool;

		auto executor_start() -> Lock;
		auto executor_wait_coroutine(Lock&& lk) -> std::pair<Lock, MessageOrStop>;
		auto executor_terminate(Lock&& lk) -> void;
		auto executor_terminate(Lock&& lk, std::exception_ptr const& e) -> void;

	private:
		std::exception_ptr executor_exception = nullptr;  // NOLINT(bugprone-throw-keyword-missing)
		std::mutex exclusion_mutex;
		std::condition_variable resume_signal;
		bool executor_running = true;
		bool executor_finished = false;
		MessageOrStop message;

		[[nodiscard]] auto is_valid_lock(Lock const& lk) const noexcept -> bool;
		auto maybe_throw(Lock&& lk) -> Lock;
	};

public:
	class Executor {
	public:
		using StopToken = Coroutine::StopToken;

		Executor() = delete;
		Executor(Executor const&) = delete;
		Executor(Executor&&) = delete;
		Executor(std::shared_ptr<Synchronizer> synchronizer) noexcept;

		auto start() -> void;
		auto wait_coroutine() -> MessageOrStop;
		auto terminate() -> void;
		auto terminate(std::exception_ptr&& e) -> void;

	private:
		std::shared_ptr<Synchronizer> synchronizer;
		Lock exclusion_lock;
	};

private:
	std::shared_ptr<Synchronizer> synchronizer;
	std::thread executor_thread;
	Lock exclusion_lock;

	auto stop_executor() -> void;
};
}  // namespace ecole::utility

#include <cassert>
#include <utility>

namespace ecole::utility {

/***********************************************
 *  Implementation of Coroutine::Synchronizer  *
 ***********************************************/

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Synchronizer::coroutine_wait_executor() -> Lock {
	Lock lk{exclusion_mutex};
	resume_signal.wait(lk, [this] { return !executor_running; });
	lk = maybe_throw(std::move(lk));
	return lk;
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Synchronizer::coroutine_resume_executor(Lock&& lk, MessageOrStop new_message) -> void {
	assert(is_valid_lock(lk));
	message = std::move(new_message);
	executor_running = true;
	lk.unlock();
	resume_signal.notify_one();
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Synchronizer::coroutine_executor_is_done([[maybe_unused]] Lock const& lk) const noexcept
	-> bool {
	assert(is_valid_lock(lk));
	return executor_finished;
}

template <typename Yield, typename Message> auto Coroutine<Yield, Message>::Synchronizer::executor_start() -> Lock {
	return Lock{exclusion_mutex};
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Synchronizer::executor_wait_coroutine(Lock&& lk) -> std::pair<Lock, MessageOrStop> {
	assert(is_valid_lock(lk));
	executor_running = false;
	lk.unlock();
	resume_signal.notify_one();
	lk.lock();
	resume_signal.wait(lk, [this] { return executor_running; });
	return {std::move(lk), std::move(message)};
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Synchronizer::executor_terminate(Lock&& lk) -> void {
	assert(is_valid_lock(lk));
	executor_running = false;
	executor_finished = true;
	lk.unlock();
	resume_signal.notify_one();
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Synchronizer::executor_terminate(Lock&& lk, std::exception_ptr const& e) -> void {
	assert(is_valid_lock(lk));
	executor_exception = e;
	executor_terminate(std::move(lk));
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Synchronizer::is_valid_lock(Lock const& lk) const noexcept -> bool {
	return lk && (lk.mutex() == &exclusion_mutex);
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Synchronizer::maybe_throw(Lock&& lk) -> Lock {
	assert(is_valid_lock(lk));
	auto e_ptr = executor_exception;
	executor_exception = nullptr;
	if (e_ptr) {
		assert(executor_finished);
		std::rethrow_exception(e_ptr);
	}
	return std::move(lk);
}

/*******************************************
 *  Implementation of Coroutine::Executor  *
 *******************************************/

template <typename Yield, typename Message>
Coroutine<Yield, Message>::Executor::Executor(std::shared_ptr<Synchronizer> synchronizer_) noexcept :
	synchronizer(std::move(synchronizer_)) {}

template <typename Yield, typename Message> auto Coroutine<Yield, Message>::Executor::start() -> void {
	exclusion_lock = synchronizer->executor_start();
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Executor::wait_coroutine() -> MessageOrStop {
	auto [lock, message] = synchronizer->executor_wait_coroutine(std::move(exclusion_lock));
	exclusion_lock = std::move(lock);
	return message;
}

template <typename Yield, typename Message> auto Coroutine<Yield, Message>::Executor::terminate() -> void {
	synchronizer->executor_terminate(std::move(exclusion_lock));
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Executor::terminate(std::exception_ptr&& except) -> void {
	synchronizer->executor_terminate(std::move(exclusion_lock), except);
}

/*********************************
 *  Implementation of Coroutine  *
 *********************************/

template <typename Yield, typename Message>
template <typename Function, typename... Args>
Coroutine<Yield, Message>::Coroutine(Function&& func_, Args&&... args_) :
	synchronizer(std::make_shared<Synchronizer>()) {
	auto executor = std::make_shared<Executor>(synchronizer);

	auto executor_func = [executor](Function&& func, Args&&... args) {
		executor->start();
		try {
			func(std::weak_ptr<Executor>(executor), std::forward<Args>(args)...);
			executor->terminate();
		} catch (...) {
			executor->terminate(std::current_exception());
		}
	};

	executor_thread = std::thread(executor_func, std::forward<Function>(func_), std::forward<Args>(args_)...);
}

template <typename Yield, typename Message> Coroutine<Yield, Message>::~Coroutine() noexcept {
	assert(std::this_thread::get_id() != executor_thread.get_id());
	if (executor_thread.joinable()) {
		try {
			stop_executor();
		} catch (...) {
			// if the Coroutine<Yield, Message> is deleted but not waited on, then we ignore potential
			// exceptions
		}
		executor_thread.join();
	}
}

template <typename Yield, typename Message> auto Coroutine<Yield, Message>::wait_executor() -> void {
	exclusion_lock = synchronizer->coroutine_wait_executor();
}

template <typename Yield, typename Message> auto Coroutine<Yield, Message>::resume_executor(Message message) -> void {
	synchronizer->coroutine_resume_executor(std::move(exclusion_lock), std::move(message));
}

template <typename Yield, typename Message>
auto Coroutine<Yield, Message>::Synchronizer::coroutine_stop_executor(Lock&& lk) -> void {
	coroutine_resume_executor(std::move(lk), Coroutine::StopToken{});
}

template <typename Yield, typename Message> auto Coroutine<Yield, Message>::is_done() const noexcept -> bool {
	return synchronizer->coroutine_executor_is_done(exclusion_lock);
}

template <typename Yield, typename Message> auto Coroutine<Yield, Message>::stop_executor() -> void {
	if (!exclusion_lock.owns_lock()) {
		exclusion_lock = synchronizer->coroutine_wait_executor();
	}
	if (!synchronizer->coroutine_executor_is_done(exclusion_lock)) {
		synchronizer->coroutine_stop_executor(std::move(exclusion_lock));
		exclusion_lock = synchronizer->coroutine_wait_executor();
	}
}

}  // namespace ecole::utility
