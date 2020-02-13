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

namespace ecole {
namespace utility {

class Controller {
public:
	using lock_t = std::unique_lock<std::mutex>;
	using action_func_t = std::function<SCIP_RETCODE(SCIP*, SCIP_RESULT*)>;

private:
	class Synchronizer {
	public:
		auto env_wait_thread() -> lock_t;
		auto env_resume_thread(lock_t&& lk, action_func_t&& action_func) -> void;
		auto env_stop_thread(lock_t&& lk) -> void;
		auto env_thread_is_done(lock_t const& lk) const noexcept -> bool;

		auto thread_start() -> lock_t;
		auto thread_hold_env(lock_t&& lk) -> lock_t;
		auto thread_terminate(lock_t&& lk) -> void;
		auto thread_terminate(lock_t&& lk, std::exception_ptr&& e) -> void;
		auto thread_action_function(lock_t const& lk) const noexcept -> action_func_t;

	private:
		std::exception_ptr except_ptr = nullptr;
		std::mutex model_mutex;
		std::condition_variable model_avail_cv;
		bool thread_owns_model = true;
		bool thread_finished = false;
		action_func_t action_func;

		auto validate_lock(lock_t const& lk) const noexcept -> void;
		auto maybe_throw(lock_t&& lk) -> lock_t;
	};

public:
	class EnvironmentInterface {
	public:
		EnvironmentInterface() = delete;
		EnvironmentInterface(EnvironmentInterface const&) = delete;
		EnvironmentInterface(EnvironmentInterface&&) = delete;

		auto wait_thread() -> void;
		auto resume_thread(action_func_t&& action_func) -> void;
		auto is_done() const noexcept -> bool;

	private:
		Synchronizer& synchronizer;
		lock_t lk;

		EnvironmentInterface(Synchronizer& synchronizer) noexcept;
		auto stop_thread() -> void;

		friend class Controller;
	};

	class ThreadInterface {
	public:
		ThreadInterface() = delete;
		ThreadInterface(ThreadInterface const&) = delete;
		ThreadInterface(ThreadInterface&&) = delete;

		auto hold_env() -> action_func_t;

	private:
		Synchronizer& synchronizer;
		lock_t lk;

		ThreadInterface(Synchronizer& synchrodnizer) noexcept;
		auto start() -> void;
		auto terminate() -> void;
		auto terminate(std::exception_ptr&& e) -> void;

		friend class Controller;
	};

	Controller();
	Controller(Controller const&) = delete;
	Controller(Controller&&) = delete;
	~Controller() noexcept;

	template <class Function, class... Args>
	static auto make_shared(Function&& f, Args&&... args) -> std::shared_ptr<Controller>;

	EnvironmentInterface& environment_interface() noexcept;
	ThreadInterface& thread_interface() noexcept;

private:
	Synchronizer synchronizer;
	EnvironmentInterface m_environment_interface;
	ThreadInterface m_thread_interface;
	std::thread solving_thread;
};

/**********************************
 *  Implementation of Controller  *
 **********************************/

template <class Function, class... Args>
auto Controller::make_shared(Function&& func, Args&&... args)
	-> std::shared_ptr<Controller> {
	auto controller = std::make_shared<Controller>();

	auto thread_func = [controller](Function&& func, Args&&... args) {
		controller->thread_interface().start();
		try {
			func(std::weak_ptr<Controller>(controller), std::forward<Args>(args)...);
			controller->thread_interface().terminate();
		} catch (...) {
			controller->thread_interface().terminate(std::current_exception());
		}
	};

	controller->solving_thread =
		std::thread(thread_func, std::forward<Function>(func), std::forward<Args>(args)...);

	return controller;
}

}  // namespace utility
}  // namespace ecole
