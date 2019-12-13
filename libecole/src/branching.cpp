#include <cassert>
#include <condition_variable>
#include <thread>

#include <scip/scip.h>

#include "ecole/branching.hpp"
#include "ecole/exception.hpp"

namespace ecole {
namespace branching {

namespace internal {

class ReverseControl::ThreadControl {
public:
	using lock_t = ReverseControl::lock_t;

	ThreadControl() = delete;
	ThreadControl(scip::Model&& model);
	// Running thread is capturing this pointer
	ThreadControl(ThreadControl&&) = delete;
	ThreadControl(ThreadControl const&) = delete;
	ThreadControl& operator=(ThreadControl&&) = delete;
	ThreadControl& operator=(ThreadControl const&) = delete;

	lock_t wait();
	void resume(scip::VarProxy var, lock_t&& lk);
	void join(lock_t&& lk);
	bool is_done(lock_t const& lk) const noexcept;
	scip::Model& get_model(lock_t const& lk) noexcept;

private:
	std::thread solve_thread;
	std::exception_ptr eptr = nullptr;
	std::mutex mut;
	std::condition_variable cv;
	bool terminate_flag = false;
	bool solve_thread_running = true;
	scip::Model model;
	scip::VarProxy branching_var = scip::VarProxy::None;

	void hold_env();
	void validate_lock(lock_t const& lk) const;
};

ReverseControl::ThreadControl::ThreadControl(scip::Model&& other_model) :
	model(std::move(other_model)) {

	auto run = [this] {
		auto branch_rule = [this](scip::Model& model) {
			hold_env();
			if (terminate_flag) model.interrupt_solve();
			return branching_var;
		};

		lock_t lk{mut};
		model.set_branch_rule(branch_rule);
		try {
			model.solve();
		} catch (...) {
			eptr = std::current_exception();
		}
		terminate_flag = true;
		solve_thread_running = false;
		lk.unlock();
		cv.notify_one();
	};

	solve_thread = std::thread(run);
}

auto ReverseControl::ThreadControl::wait() -> lock_t {
	assert(solve_thread.joinable());

	lock_t lk{mut};
	cv.wait(lk, [this] { return !solve_thread_running; });
	if (eptr) std::rethrow_exception(eptr);
	return lk;
}

void ReverseControl::ThreadControl::resume(scip::VarProxy var, lock_t&& lk) {
	validate_lock(lk);
	branching_var = var;
	solve_thread_running = true;
	lk.unlock();
	cv.notify_one();
}

void ReverseControl::ThreadControl::join(lock_t&& lk) {
	if (solve_thread.joinable()) {
		if (!lk.owns_lock()) lk = wait();
		validate_lock(lk);
		if (!terminate_flag) {
			terminate_flag = true;
			resume(scip::VarProxy::None, std::move(lk));
			lk = wait();  // Get eventual exception
		}
		solve_thread.join();
	}
}

bool ReverseControl::ThreadControl::is_done(lock_t const& lk) const noexcept {
	validate_lock(lk);
	return terminate_flag;
}

scip::Model& ReverseControl::ThreadControl::get_model(lock_t const& lk) noexcept {
	validate_lock(lk);
	return model;
}

void ReverseControl::ThreadControl::hold_env() {
	lock_t lk{mut, std::adopt_lock};
	solve_thread_running = false;
	lk.unlock();
	cv.notify_one();
	lk.lock();
	cv.wait(lk, [this] { return solve_thread_running; });
	lk.release();
}

void ReverseControl::ThreadControl::validate_lock(lock_t const& lk) const {
	(void)lk;
	assert(lk && (lk.mutex() == &mut));
}

ReverseControl::ReverseControl() noexcept = default;
ReverseControl::ReverseControl(ReverseControl&&) = default;

ReverseControl::ReverseControl(scip::Model&& model) :
	thread_control(std::make_unique<ThreadControl>(std::move(model))),
	lk_ptr(std::make_unique<lock_t>()) {}

ReverseControl& ReverseControl::operator=(ReverseControl&& other) {
	if (thread_control) thread_control->join(std::move(*lk_ptr));
	thread_control = std::move(other.thread_control);
	lk_ptr = std::move(other.lk_ptr);
	return *this;
}

ReverseControl::~ReverseControl() {
	if (thread_control) thread_control->join(std::move(*lk_ptr));
}

void ReverseControl::wait() {
	assert(thread_control);
	*lk_ptr = thread_control->wait();
}

void ReverseControl::resume(scip::VarProxy var) {
	assert(thread_control);
	assert(lk_ptr);
	thread_control->resume(var, std::move(*lk_ptr));
}

bool ReverseControl::is_done() const noexcept {
	return thread_control->is_done(*lk_ptr);
}

scip::Model& ReverseControl::model() noexcept {
	return thread_control->get_model(*lk_ptr);
}

}  // namespace internal

scip::VarProxy Fractional::get(scip::Model& model, std::size_t const& action) {
	return model.lp_branch_vars().at(action);
}

auto Fractional::clone() const -> std::unique_ptr<ActionSpace> {
	return std::make_unique<Fractional>(*this);
}

}  // namespace branching
}  // namespace ecole
