#include <cassert>
#include <condition_variable>
#include <memory.h>
#include <thread>

#include <scip/scip.h>

#include "ecole/branching.hpp"
#include "ecole/exception.hpp"

namespace ecole {
namespace branching {

namespace internal {

template <template <typename...> class Holder>
class ReverseControl<Holder>::ThreadControl {
public:
	using lock_t = ReverseControl::lock_t;

	template <typename T> using ptr = Holder<T>;

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
	scip::VarProxy branching_var = scip::VarProxy::None;
	ptr<scip::Model> model;

	void hold_env();
	void validate_lock(lock_t const& lk) const;
};

template <template <typename...> class H>
ReverseControl<H>::ThreadControl::ThreadControl(scip::Model&& other_model) :
	model(std::make_unique<scip::Model>(std::move(other_model))) {

	auto run = [this] {
		auto branch_rule = [this](scip::Model& model) {
			hold_env();
			if (terminate_flag) model.interrupt_solve();
			return branching_var;
		};

		lock_t lk{mut};
		get_model(lk).set_branch_rule(branch_rule);
		try {
			get_model(lk).solve();
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

template <template <typename...> class H>
auto ReverseControl<H>::ThreadControl::wait() -> lock_t {
	assert(solve_thread.joinable());

	lock_t lk{mut};
	cv.wait(lk, [this] { return !solve_thread_running; });
	if (eptr) std::rethrow_exception(eptr);
	return lk;
}

template <template <typename...> class H>
void ReverseControl<H>::ThreadControl::resume(scip::VarProxy var, lock_t&& lk) {
	validate_lock(lk);
	branching_var = var;
	solve_thread_running = true;
	lk.unlock();
	cv.notify_one();
}

template <template <typename...> class H>
void ReverseControl<H>::ThreadControl::join(lock_t&& lk) {
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

template <template <typename...> class H>
bool ReverseControl<H>::ThreadControl::is_done(lock_t const& lk) const noexcept {
	validate_lock(lk);
	return terminate_flag;
}

template <template <typename...> class H>
scip::Model& ReverseControl<H>::ThreadControl::get_model(lock_t const& lk) noexcept {
	validate_lock(lk);
	return *model;
}

template <template <typename...> class H>
void ReverseControl<H>::ThreadControl::hold_env() {
	lock_t lk{mut, std::adopt_lock};
	solve_thread_running = false;
	lk.unlock();
	cv.notify_one();
	lk.lock();
	cv.wait(lk, [this] { return solve_thread_running; });
	lk.release();
}

template <template <typename...> class H>
void ReverseControl<H>::ThreadControl::validate_lock(lock_t const& lk) const {
	(void)lk;
	assert(lk && (lk.mutex() == &mut));
}

template <template <typename...> class H>
ReverseControl<H>::ReverseControl() noexcept = default;
template <template <typename...> class H>
ReverseControl<H>::ReverseControl(ReverseControl&&) = default;

template <template <typename...> class H>
ReverseControl<H>::ReverseControl(scip::Model&& model) :
	thread_control(std::make_unique<ThreadControl>(std::move(model))),
	lk_ptr(std::make_unique<lock_t>()) {}

template <template <typename...> class H>
auto ReverseControl<H>::operator=(ReverseControl&& other) -> ReverseControl& {
	if (thread_control) thread_control->join(std::move(*lk_ptr));
	thread_control = std::move(other.thread_control);
	lk_ptr = std::move(other.lk_ptr);
	return *this;
}

template <template <typename...> class H> ReverseControl<H>::~ReverseControl() {
	if (thread_control) thread_control->join(std::move(*lk_ptr));
}

template <template <typename...> class H> void ReverseControl<H>::wait() {
	assert(thread_control);
	*lk_ptr = thread_control->wait();
}

template <template <typename...> class H>
void ReverseControl<H>::resume(scip::VarProxy var) {
	assert(thread_control);
	assert(lk_ptr);
	thread_control->resume(var, std::move(*lk_ptr));
}

template <template <typename...> class H>
bool ReverseControl<H>::is_done() const noexcept {
	return thread_control->is_done(*lk_ptr);
}

template <template <typename...> class H>
scip::Model& ReverseControl<H>::model() noexcept {
	return thread_control->get_model(*lk_ptr);
}

template class ReverseControl<std::unique_ptr>;
template class ReverseControl<std::shared_ptr>;

}  // namespace internal

scip::VarProxy Fractional::get(scip::Model& model, std::size_t const& action) {
	return model.lp_branch_vars().at(action);
}

auto Fractional::clone() const -> std::unique_ptr<ActionSpace> {
	return std::make_unique<Fractional>(*this);
}

}  // namespace branching
}  // namespace ecole
