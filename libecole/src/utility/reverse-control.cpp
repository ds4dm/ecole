#include <utility>

#include "ecole/utility/reverse-control.hpp"

namespace ecole::utility {

/************************************************
 *  Implementation of Controller::Synchronizer  *
 ************************************************/

auto Controller::Synchronizer::env_wait_thread() -> lock_t {
	lock_t lk{model_mutex};
	model_avail_cv.wait(lk, [this] { return !thread_owns_model; });
	lk = maybe_throw(std::move(lk));
	return lk;
}

auto Controller::Synchronizer::env_resume_thread(lock_t&& lk, action_func_t&& new_action_func) -> void {
	assert(is_valid_lock(lk));
	action_func = std::move(new_action_func);
	thread_owns_model = true;
	lk.unlock();
	model_avail_cv.notify_one();
}

auto Controller::Synchronizer::env_stop_thread(lock_t&& lk) -> void {
	assert(is_valid_lock(lk));
	if (!thread_finished) {
		env_resume_thread(std::move(lk), [](auto* scip, auto* result) {
			SCIP_CALL(SCIPinterruptSolve(scip));
			*result = SCIP_DIDNOTRUN;
			return SCIP_OKAY;
		});
		lk = env_wait_thread();
		assert(is_valid_lock(lk));
	}
	lk = maybe_throw(std::move(lk));
}

auto Controller::Synchronizer::env_thread_is_done([[maybe_unused]] lock_t const& lk) const noexcept -> bool {
	assert(is_valid_lock(lk));
	return thread_finished;
}

auto Controller::Synchronizer::thread_start() -> lock_t {
	return lock_t{model_mutex};
}

auto Controller::Synchronizer::thread_hold_env(lock_t&& lk) -> lock_t {
	assert(is_valid_lock(lk));
	thread_owns_model = false;
	lk.unlock();
	model_avail_cv.notify_one();
	lk.lock();
	model_avail_cv.wait(lk, [this] { return thread_owns_model; });
	return std::move(lk);
}

auto Controller::Synchronizer::thread_terminate(lock_t&& lk) -> void {
	assert(is_valid_lock(lk));
	thread_owns_model = false;
	thread_finished = true;
	lk.unlock();
	model_avail_cv.notify_one();
}

auto Controller::Synchronizer::thread_terminate(lock_t&& lk, std::exception_ptr const& e) -> void {
	assert(is_valid_lock(lk));
	except_ptr = e;
	thread_terminate(std::move(lk));
}

auto Controller::Synchronizer::thread_action_function([[maybe_unused]] lock_t const& lk) const noexcept
	-> action_func_t {
	assert(is_valid_lock(lk));
	return action_func;
}

auto Controller::Synchronizer::is_valid_lock(lock_t const& lk) const noexcept -> bool {
	return lk && (lk.mutex() == &model_mutex);
}

auto Controller::Synchronizer::maybe_throw(lock_t&& lk) -> lock_t {
	assert(is_valid_lock(lk));
	auto e_ptr = except_ptr;
	except_ptr = nullptr;
	if (e_ptr) {
		assert(thread_finished);
		std::rethrow_exception(e_ptr);
	}
	return std::move(lk);
}

/********************************************
 *  Implementation of Controller::Executor  *
 ********************************************/

Controller::Executor::Executor(std::shared_ptr<Synchronizer> synchronizer_) noexcept :
	synchronizer(std::move(synchronizer_)) {}

auto Controller::Executor::start() -> void {
	model_lock = synchronizer->thread_start();
}

auto Controller::Executor::hold_env() -> action_func_t {
	model_lock = synchronizer->thread_hold_env(std::move(model_lock));
	return synchronizer->thread_action_function(model_lock);
}

auto Controller::Executor::terminate() -> void {
	synchronizer->thread_terminate(std::move(model_lock));
}

auto Controller::Executor::terminate(std::exception_ptr&& except) -> void {
	synchronizer->thread_terminate(std::move(model_lock), except);
}

/**********************************
 *  Implementation of Controller  *
 **********************************/

Controller::~Controller() noexcept {
	assert(std::this_thread::get_id() != solving_thread.get_id());
	if (solving_thread.joinable()) {
		try {
			stop_thread();
		} catch (...) {
			// if the Controller is deleted but not waited on, then we ignore potential
			// exceptions
		}
		solving_thread.join();
	}
}

auto Controller::wait_thread() -> void {
	model_lock = synchronizer->env_wait_thread();
}

auto Controller::resume_thread(action_func_t&& action_func) -> void {
	synchronizer->env_resume_thread(std::move(model_lock), std::move(action_func));
}

auto Controller::is_done() const noexcept -> bool {
	return synchronizer->env_thread_is_done(model_lock);
}

auto Controller::stop_thread() -> void {
	if (!model_lock.owns_lock()) {
		model_lock = synchronizer->env_wait_thread();
	}
	synchronizer->env_stop_thread(std::move(model_lock));
}

}  // namespace ecole::utility
