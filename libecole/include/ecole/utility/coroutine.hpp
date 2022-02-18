#pragma once

#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <variant>

namespace ecole::utility {

/**
 * Asynchronous cooperative interruptable code execution.
 *
 * Asynchronously execute a piece of code in an interative fashion while producing intermediary results.
 * User-defined messages can be send to communicate with the executor.
 * The execution flow is as follow:
 * 1. Upon creation, the instruction provided in the constructor start being executed by the executor.
 * 2. The ``yield`` function is called by the executor with the first return value.
 * 3. The coroutine calls ``wait`` to recieve that first return value.
 * 4. If no value is returned, then the executor has finished.
 * 5. Else, the coroutine calls ``resume`` with a message to pass to the executor.
 * 6. The executor recieve the message and continue its execution unitl the next ``yield``, the process repeats from 2.
 *
 * @tparam Return The type of return values created by the executor.
 * @tparam Message The type of the messages that can be sent to the executor.
 */
template <typename Return, typename Message> class Coroutine {
public:
	/** Return or nothing if the corutine has finished. */
	using MaybeReturn = std::optional<Return>;

	/**
	 * Start the execution.
	 *
	 * @param func Function used to define the code that needs to be executed by the executor.
	 *        The first parameter to that function is a ``std::weak_ptr<Executor>`` used to yield values and recieve
	 *        messages.
	 *        If the weak pointer is expired, the executor must terminate.
	 * @param args Additional parameters to be passed as additinal arguments to ``func``.
	 */
	template <class Function, class... Args> Coroutine(Function&& func, Args&&... args);

	/**
	 * Terminate the coroutine
	 *
	 * The destructor can safely be called at anytime.
	 * If the executor is still running, it will recieve a ``StopToken`` and must terminate, otherwise the whole program
	 * will itself terminate.
	 * Any return value left to be yielded by the executor will be lost.
	 *
	 * @see StopToken
	 */
	~Coroutine() noexcept;

	/**
	 * Wait for the executor to yield a value.
	 *
	 * If no return value is given, then the coroutine has finished.
	 * This function should not be called successively without calling ``resume``.
	 */
	auto wait() -> MaybeReturn;

	/**
	 * Send a message and resume the executor.
	 *
	 * This function should not be called without first calling ``wait``, or if ``wait`` has not returned a value.
	 */
	auto resume(Message instruction) -> void;

private:
	/** Type indicating that the executor must terminate. */
	struct StopToken {};

	/** Message recieved by the executor. */
	using MessageOrStop = std::variant<Message, Coroutine::StopToken>;

	/**
	 * Lock type to synchronise between the coroutine and executor.
	 *
	 * The lock is passed back and forth to guarantee that it is help/release when necessary.
	 */
	using Lock = std::unique_lock<std::mutex>;

	/** Class responsible for synchronizing between the coroutine and executor. */
	class Synchronizer {
	public:
		auto coroutine_wait_executor() -> Lock;
		auto coroutine_pop_return() -> Return;
		auto coroutine_resume_executor(Lock&& lk, MessageOrStop instruction) -> void;
		auto coroutine_stop_executor(Lock&& lk) -> void;
		[[nodiscard]] auto coroutine_executor_is_done(Lock const& lk) const noexcept -> bool;

		auto executor_start() -> Lock;
		auto executor_yield(Lock&& lk, Return value) -> std::pair<Lock, MessageOrStop>;
		auto executor_terminate(Lock&& lk) -> void;
		auto executor_terminate(Lock&& lk, std::exception_ptr const& e) -> void;

	private:
		std::exception_ptr m_executor_exception = nullptr;  // NOLINT(bugprone-throw-keyword-missing)
		std::mutex m_exclusion_mutex;
		std::condition_variable m_resume_signal;
		bool m_executor_running = true;
		bool m_executor_finished = false;
		Return m_value;
		MessageOrStop m_instruction;

		[[nodiscard]] auto is_valid_lock(Lock const& lk) const noexcept -> bool;
		auto maybe_throw(Lock&& lk) -> Lock;
	};

public:
	/** Class to communicate with the coroutine from the executor. */
	class Executor {
	public:
		/** Type indicating that the executor must terminate. */
		using StopToken = Coroutine::StopToken;
		/** Message recieved by the executor. */
		using MessageOrStop = Coroutine::MessageOrStop;

		/** Return whether the message is a ``StopToken``/ */
		static auto is_stop(MessageOrStop const& message) -> bool;

		Executor() = delete;
		Executor(Executor const&) = delete;
		Executor(Executor&&) = delete;
		Executor(std::shared_ptr<Synchronizer> synchronizer) noexcept;

		/**
		 * Yield a value, and wait to be recieve a message from the coroutine.
		 *
		 * If instead of a message, the executor recieves a ``StopToken``, then it must terminate.
		 */
		auto yield(Return value) -> MessageOrStop;

	private:
		std::shared_ptr<Synchronizer> m_synchronizer;
		Lock m_exclusion_lock;

		friend class Coroutine;
		/** Indicate to the synchronizer that executor is ready to start. */
		auto start() -> void;
		/** Indicate to the synchronizer that executor has terminated without error. */
		auto terminate() -> void;
		/** Indicate to the synchronizer that executor has terminated with an error. */
		auto terminate(std::exception_ptr&& e) -> void;
	};

private:
	std::shared_ptr<Synchronizer> m_synchronizer;
	std::thread executor_thread;
	Lock m_exclusion_lock;

	auto stop_executor() -> void;
};
}  // namespace ecole::utility

#include <cassert>
#include <type_traits>
#include <utility>

#include "ecole/utility/function-traits.hpp"

namespace ecole::utility {

/***********************************************
 *  Implementation of Coroutine::Synchronizer  *
 ***********************************************/

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::coroutine_wait_executor() -> Lock {
	Lock lk{m_exclusion_mutex};
	m_resume_signal.wait(lk, [this] { return !m_executor_running; });
	return maybe_throw(std::move(lk));
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::coroutine_pop_return() -> Return {
	return std::move(m_value);
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::coroutine_resume_executor(Lock&& lk, MessageOrStop new_instruction)
	-> void {
	assert(is_valid_lock(lk));
	m_instruction = std::move(new_instruction);
	m_executor_running = true;
	lk.unlock();
	m_resume_signal.notify_one();
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::coroutine_executor_is_done(
	[[maybe_unused]] Lock const& lk) const noexcept -> bool {
	assert(is_valid_lock(lk));
	return m_executor_finished;
}

template <typename Return, typename Message> auto Coroutine<Return, Message>::Synchronizer::executor_start() -> Lock {
	return Lock{m_exclusion_mutex};
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::executor_yield(Lock&& lk, Return value)
	-> std::pair<Lock, MessageOrStop> {
	assert(is_valid_lock(lk));
	m_executor_running = false;
	m_value = value;
	lk.unlock();
	m_resume_signal.notify_one();
	lk.lock();
	m_resume_signal.wait(lk, [this] { return m_executor_running; });
	return {std::move(lk), std::move(m_instruction)};
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::executor_terminate(Lock&& lk) -> void {
	assert(is_valid_lock(lk));
	m_executor_running = false;
	m_executor_finished = true;
	lk.unlock();
	m_resume_signal.notify_one();
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::executor_terminate(Lock&& lk, std::exception_ptr const& e) -> void {
	assert(is_valid_lock(lk));
	m_executor_exception = e;
	executor_terminate(std::move(lk));
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::is_valid_lock(Lock const& lk) const noexcept -> bool {
	return lk && (lk.mutex() == &m_exclusion_mutex);
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::maybe_throw(Lock&& lk) -> Lock {
	assert(is_valid_lock(lk));
	auto e_ptr = m_executor_exception;
	m_executor_exception = nullptr;
	if (e_ptr) {
		assert(m_executor_finished);
		std::rethrow_exception(e_ptr);
	}
	return std::move(lk);
}

/*******************************************
 *  Implementation of Coroutine::Executor  *
 *******************************************/

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Executor::is_stop(MessageOrStop const& message) -> bool {
	return std::holds_alternative<StopToken>(message);
}

template <typename Return, typename Message>
Coroutine<Return, Message>::Executor::Executor(std::shared_ptr<Synchronizer> synchronizer) noexcept :
	m_synchronizer(std::move(synchronizer)) {}

template <typename Return, typename Message> auto Coroutine<Return, Message>::Executor::start() -> void {
	m_exclusion_lock = m_synchronizer->executor_start();
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Executor::yield(Return value) -> MessageOrStop {
	auto [lock, instruction] = m_synchronizer->executor_yield(std::move(m_exclusion_lock), std::move(value));
	m_exclusion_lock = std::move(lock);
	return instruction;
}

template <typename Return, typename Message> auto Coroutine<Return, Message>::Executor::terminate() -> void {
	m_synchronizer->executor_terminate(std::move(m_exclusion_lock));
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Executor::terminate(std::exception_ptr&& except) -> void {
	m_synchronizer->executor_terminate(std::move(m_exclusion_lock), except);
}

/*********************************
 *  Implementation of Coroutine  *
 *********************************/

template <typename Return, typename Message>
template <typename Function, typename... Args>
Coroutine<Return, Message>::Coroutine(Function&& func_, Args&&... args_) :
	m_synchronizer(std::make_shared<Synchronizer>()) {
	auto executor = std::make_shared<Executor>(m_synchronizer);

	auto executor_func = [executor](Function&& func, Args&&... args) {
		executor->start();
		try {
			using ExecutorArg = std::remove_const_t<std::remove_reference_t<utility::arg_t<0, Function>>>;
			if constexpr (std::is_same_v<ExecutorArg, std::shared_ptr<Executor>>) {
				func(executor, std::forward<Args>(args)...);
			} else if constexpr (std::is_same_v<ExecutorArg, std::weak_ptr<Executor>>) {
				func(std::weak_ptr<Executor>(executor), std::forward<Args>(args)...);
			} else {
				func(*executor, std::forward<Args>(args)...);
			}
			executor->terminate();
		} catch (...) {
			executor->terminate(std::current_exception());
		}
	};

	executor_thread = std::thread(executor_func, std::forward<Function>(func_), std::forward<Args>(args_)...);
}

template <typename Return, typename Message> Coroutine<Return, Message>::~Coroutine() noexcept {
	assert(std::this_thread::get_id() != executor_thread.get_id());
	if (executor_thread.joinable()) {
		try {
			stop_executor();
		} catch (...) {
			// if the Coroutine<Return, Message> is deleted but not waited on, then we ignore potential
			// exceptions
		}
		executor_thread.join();
	}
}

template <typename Return, typename Message> auto Coroutine<Return, Message>::wait() -> MaybeReturn {
	m_exclusion_lock = m_synchronizer->coroutine_wait_executor();
	if (m_synchronizer->coroutine_executor_is_done(m_exclusion_lock)) {
		return std::nullopt;
	}
	return m_synchronizer->coroutine_pop_return();
}

template <typename Return, typename Message> auto Coroutine<Return, Message>::resume(Message instruction) -> void {
	m_synchronizer->coroutine_resume_executor(std::move(m_exclusion_lock), std::move(instruction));
}

template <typename Return, typename Message>
auto Coroutine<Return, Message>::Synchronizer::coroutine_stop_executor(Lock&& lk) -> void {
	coroutine_resume_executor(std::move(lk), Coroutine::StopToken{});
}

template <typename Return, typename Message> auto Coroutine<Return, Message>::stop_executor() -> void {
	if (!m_exclusion_lock.owns_lock()) {
		m_exclusion_lock = m_synchronizer->coroutine_wait_executor();
	}
	// Could be an `if` statement because executors are supposed to terminate directly when being sent a StopToken.
	// However, using coroutine with multiple SCIP callbacks, some callbacks might still be called even after
	// `SCIPinterrupt` is called on `StopToken`.
	while (!m_synchronizer->coroutine_executor_is_done(m_exclusion_lock)) {
		m_synchronizer->coroutine_stop_executor(std::move(m_exclusion_lock));
		m_exclusion_lock = m_synchronizer->coroutine_wait_executor();
	}
}

}  // namespace ecole::utility
