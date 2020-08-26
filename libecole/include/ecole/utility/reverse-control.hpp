#pragma once

#include <cassert>
#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include <scip/scip.h>

namespace ecole::utility {

class Controller {
public:
	using lock_t = std::unique_lock<std::mutex>;
	using action_func_t = std::function<SCIP_RETCODE(SCIP*, SCIP_RESULT*)>;

	Controller() = default;
	template <class Function, class... Args> Controller(Function&& func, Args&&... args);
	~Controller() noexcept;

	auto wait_thread() -> void;
	auto resume_thread(action_func_t&& action_func) -> void;
	[[nodiscard]] auto is_done() const noexcept -> bool;

private:
	class Synchronizer {
	public:
		auto env_wait_thread() -> lock_t;
		auto env_resume_thread(lock_t&& lk, action_func_t&& action_func) -> void;
		auto env_stop_thread(lock_t&& lk) -> void;
		[[nodiscard]] auto env_thread_is_done(lock_t const& lk) const noexcept -> bool;

		auto thread_start() -> lock_t;
		auto thread_hold_env(lock_t&& lk) -> lock_t;
		auto thread_terminate(lock_t&& lk) -> void;
		auto thread_terminate(lock_t&& lk, std::exception_ptr const& e) -> void;
		[[nodiscard]] auto thread_action_function(lock_t const& lk) const noexcept -> action_func_t;

	private:
		std::exception_ptr except_ptr = nullptr;  // NOLINT(bugprone-throw-keyword-missing)
		std::mutex model_mutex;
		std::condition_variable model_avail_cv;
		bool thread_owns_model = true;
		bool thread_finished = false;
		action_func_t action_func;

		[[nodiscard]] auto is_valid_lock(lock_t const& lk) const noexcept -> bool;
		auto maybe_throw(lock_t&& lk) -> lock_t;
	};

public:
	class Executor {
	public:
		Executor() = delete;
		Executor(Executor const&) = delete;
		Executor(Executor&&) = delete;
		Executor(std::shared_ptr<Synchronizer> synchronizer) noexcept;

		auto start() -> void;
		auto hold_env() -> action_func_t;
		auto terminate() -> void;
		auto terminate(std::exception_ptr&& e) -> void;

	private:
		std::shared_ptr<Synchronizer> synchronizer;
		lock_t model_lock;
	};

private:
	std::shared_ptr<Synchronizer> synchronizer;
	std::thread solving_thread;
	lock_t model_lock;

	auto stop_thread() -> void;
};

/**********************************
 *  Implementation of Controller  *
 **********************************/

template <class Function, class... Args>
Controller::Controller(Function&& func_, Args&&... args_) : synchronizer(std::make_shared<Synchronizer>()) {
	auto executor = std::make_shared<Executor>(synchronizer);

	auto thread_func = [executor](Function&& func, Args&&... args) {
		executor->start();
		try {
			func(std::weak_ptr<Executor>(executor), std::forward<Args>(args)...);
			executor->terminate();
		} catch (...) {
			executor->terminate(std::current_exception());
		}
	};

	solving_thread = std::thread(thread_func, std::forward<Function>(func_), std::forward<Args>(args_)...);
}

}  // namespace ecole::utility
