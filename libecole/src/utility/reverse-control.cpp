#include "utility/reverse-control.hpp"

namespace ecole {
namespace utility {

/************************************************
 *  Implementation of Controller::Synchronizer  *
 ************************************************/

auto Controller::Synchronizer::env_wait_thread() -> lock_t {
	lock_t lk{model_mutex};
	model_avail_cv.wait(lk, [this] { return !thread_owns_model; });
	lk = maybe_throw(std::move(lk));
	return lk;
}

auto Controller::Synchronizer::env_resume_thread(
	lock_t&& lk,
	action_func_t&& new_action_func) -> void {
	validate_lock(lk);
	action_func = std::move(new_action_func);
	thread_owns_model = true;
	lk.unlock();
	model_avail_cv.notify_one();
}

auto Controller::Synchronizer::env_stop_thread(lock_t&& lk) -> void {
	validate_lock(lk);
	if (!thread_finished) {
		env_resume_thread(std::move(lk), [](auto* scip, auto* result) {
			SCIP_CALL(SCIPinterruptSolve(scip));
			*result = SCIP_DIDNOTRUN;
			return SCIP_OKAY;
		});
		lk = env_wait_thread();
		validate_lock(lk);
	}
	lk = maybe_throw(std::move(lk));
}

auto Controller::Synchronizer::env_thread_is_done(lock_t const& lk) const noexcept
	-> bool {
	validate_lock(lk);
	return thread_finished;
}

auto Controller::Synchronizer::thread_start() -> lock_t {
	return lock_t{model_mutex};
}

auto Controller::Synchronizer::thread_hold_env(lock_t&& lk) -> lock_t {
	validate_lock(lk);
	thread_owns_model = false;
	lk.unlock();
	model_avail_cv.notify_one();
	lk.lock();
	model_avail_cv.wait(lk, [this] { return thread_owns_model; });
	return std::move(lk);
}

auto Controller::Synchronizer::thread_terminate(lock_t&& lk) -> void {
	validate_lock(lk);
	thread_owns_model = false;
	thread_finished = true;
	lk.unlock();
	model_avail_cv.notify_one();
}

auto Controller::Synchronizer::thread_terminate(lock_t&& lk, std::exception_ptr&& e)
	-> void {
	validate_lock(lk);
	except_ptr = std::move(e);
	thread_terminate(std::move(lk));
}

auto Controller::Synchronizer::thread_action_function(lock_t const& lk) const noexcept
	-> action_func_t {
	validate_lock(lk);
	return std::move(action_func);
}

auto Controller::Synchronizer::validate_lock(lock_t const& lk) const noexcept -> void {
	(void)lk;
	assert(lk && (lk.mutex() == &model_mutex));
}

auto Controller::Synchronizer::maybe_throw(lock_t&& lk) -> lock_t {
	validate_lock(lk);
	auto e_ptr = std::move(except_ptr);
	except_ptr = nullptr;
	if (e_ptr) {
		assert(thread_finished);
		std::rethrow_exception(std::move(e_ptr));
	} else
		return std::move(lk);
}

/********************************************************
 *  Implementation of Controller::EnvironmentInterface  *
 ********************************************************/

auto Controller::EnvironmentInterface::wait_thread() -> void {
	lk = synchronizer.env_wait_thread();
}

auto Controller::EnvironmentInterface::resume_thread(action_func_t&& action_func)
	-> void {
	synchronizer.env_resume_thread(std::move(lk), std::move(action_func));
}

auto Controller::EnvironmentInterface::is_done() const noexcept -> bool {
	return synchronizer.env_thread_is_done(lk);
}

Controller::EnvironmentInterface::EnvironmentInterface(
	Synchronizer& synchronizer) noexcept :
	synchronizer(synchronizer) {}

auto Controller::EnvironmentInterface::stop_thread() -> void {
	if (!lk.owns_lock()) lk = synchronizer.env_wait_thread();
	synchronizer.env_stop_thread(std::move(lk));
}

/***************************************
 *  Implementation of Controller::ThreadInterface  *
 ***************************************/

auto Controller::ThreadInterface::hold_env() -> action_func_t {
	lk = synchronizer.thread_hold_env(std::move(lk));
	return synchronizer.thread_action_function(lk);
}

Controller::ThreadInterface::ThreadInterface(Synchronizer& synchronizer) noexcept :
	synchronizer(synchronizer) {}

auto Controller::ThreadInterface::start() -> void {
	lk = synchronizer.thread_start();
}

auto Controller::ThreadInterface::terminate() -> void {
	synchronizer.thread_terminate(std::move(lk));
}

auto Controller::ThreadInterface::terminate(std::exception_ptr&& except) -> void {
	synchronizer.thread_terminate(std::move(lk), std::move(except));
}

/**********************************
 *  Implementation of Controller  *
 **********************************/

Controller::~Controller() noexcept {
	try {
		if (solving_thread.joinable()) {
			environment_interface().stop_thread();
			solving_thread.join();
		}
	} catch (...) {
		// if the Controller is deleted bu not waited on, then we ignore potential exceptions
	}
}

auto Controller::environment_interface() noexcept -> EnvironmentInterface& {
	return m_environment_interface;
}

auto Controller::thread_interface() noexcept -> ThreadInterface& {
	return m_thread_interface;
}

Controller::Controller() :
	synchronizer(),
	m_environment_interface(synchronizer),
	m_thread_interface(synchronizer) {}

}  // namespace utility
}  // namespace ecole
