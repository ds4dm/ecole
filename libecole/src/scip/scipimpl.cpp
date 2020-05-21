#include <mutex>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "ecole/scip/scipimpl.hpp"

#include "scip/utils.hpp"

namespace ecole {
namespace scip {

void ScipDeleter::operator()(SCIP* ptr) {
	scip::call(SCIPfree, &ptr);
}

static std::unique_ptr<SCIP, ScipDeleter> create_scip() {
	SCIP* scip_raw;
	scip::call(SCIPcreate, &scip_raw);
	SCIPmessagehdlrSetQuiet(SCIPgetMessagehdlr(scip_raw), true);
	std::unique_ptr<SCIP, ScipDeleter> scip_ptr = nullptr;
	scip_ptr.reset(scip_raw);
	return scip_ptr;
}

static std::unique_ptr<SCIP, ScipDeleter> copy_orig(SCIP const* const source) {
	if (!source) return nullptr;
	if (SCIPgetStage(const_cast<SCIP*>(source)) == SCIP_STAGE_INIT) return create_scip();
	auto dest = create_scip();
	// Copy operation is not thread safe
	static std::mutex m{};
	std::lock_guard<std::mutex> g{m};
	scip::call(
		SCIPcopyOrig,
		const_cast<SCIP*>(source),
		dest.get(),
		nullptr,
		nullptr,
		"",
		false,
		false,
		false,
		nullptr);
	return dest;
}

scip::ScipImpl::ScipImpl() : m_scip(create_scip()) {
	scip::call(SCIPincludeDefaultPlugins, get_scip_ptr());
}

ScipImpl::ScipImpl(std::unique_ptr<SCIP, ScipDeleter>&& scip_ptr) noexcept :
	m_scip(std::move(scip_ptr)) {}

SCIP* scip::ScipImpl::get_scip_ptr() noexcept {
	return m_scip.get();
}

scip::ScipImpl scip::ScipImpl::copy_orig() {
	return ::ecole::scip::copy_orig(get_scip_ptr());
}

}  // namespace scip
}  // namespace ecole
