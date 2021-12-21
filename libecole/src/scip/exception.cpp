#include <cstddef>
#include <exception>
#include <iostream>
#include <memory.h>
#include <memory>
#include <string>
#include <utility>

#include <objscip/objmessagehdlr.h>

#include "ecole/scip/exception.hpp"

namespace ecole::scip {

/***********************************
 *  Declaration of ErrorCollector  *
 ***********************************/

namespace {

/**
 * SCIP message handler to collect error messages.
 *
 * This message handler stores error message rather than printing them to standard error.
 * The messages can then be collected by the exception maker to craft the messsage of the exception.
 *
 * The class stores the messages in a thread local static variable.
 */
class ErrorCollector : public ::scip::ObjMessagehdlr {
public:
	static std::string collect();
	static void clear();

	ErrorCollector() noexcept;

	void scip_error(SCIP_MESSAGEHDLR* messagehdlr, FILE* file, const char* message) override;

private:
	thread_local static std::string errors;
	constexpr static std::size_t buffer_size = 1000;
};

/**
 * Deleter type for SCIP_MESSAHEHDLR to use with unique_ptr.
 */
struct MessageHandlerDeleter {
	void operator()(SCIP_MESSAGEHDLR* ptr);
};

/**
 * The SCIP_MESSAGE_HANDLER that wraps the Error Collector.
 *
 * SCIP takes a ObjMessage and allocate a SCIP_MESSAGE_HANDLER that contains it.
 * We are responsible for dealocating it, hence the unique_ptr.
 */
extern std::unique_ptr<SCIP_MESSAGEHDLR, MessageHandlerDeleter> message_handler;

}  // namespace

/*****************************
 *  Definition of ScipError  *
 *****************************/

ScipError ScipError::from_retcode(SCIP_RETCODE retcode) {
	auto message = ErrorCollector::collect();
	if (!message.empty()) {
		return ScipError{std::move(message)};
	}
	switch (retcode) {
	case SCIP_OKAY:
		throw ScipError{"Normal termination must not raise exception"};
	case SCIP_ERROR:
		return ScipError{"Unspecified error"};
	case SCIP_NOMEMORY:
		return ScipError{"Insufficient memory error"};
	case SCIP_READERROR:
		return ScipError{"File read error"};
	case SCIP_WRITEERROR:
		return ScipError{"File write error"};
	case SCIP_BRANCHERROR:
		return ScipError{"Branch error"};
	case SCIP_NOFILE:
		return ScipError{"File not found error"};
	case SCIP_FILECREATEERROR:
		return ScipError{"Cannot create file"};
	case SCIP_LPERROR:
		return ScipError{"Error in LP solver"};
	case SCIP_NOPROBLEM:
		return ScipError{"No problem exists"};
	case SCIP_INVALIDCALL:
		return ScipError{"Method cannot be called at tScipError(his time in solution process"};
	case SCIP_INVALIDDATA:
		return ScipError{"Method cannot be called with this type of data"};
	case SCIP_INVALIDRESULT:
		return ScipError{"Method returned an invalid result code"};
	case SCIP_PLUGINNOTFOUND:
		return ScipError{"A required plugin was not found"};
	case SCIP_PARAMETERUNKNOWN:
		return ScipError{"The parameter with the given name was not found"};
	case SCIP_PARAMETERWRONGTYPE:
		return ScipError{"The parameter is not of the expected type"};
	case SCIP_PARAMETERWRONGVAL:
		return ScipError{"The value is invalid for the given parameter"};
	case SCIP_KEYALREADYEXISTING:
		return ScipError{"The given key is already existing in table"};
	case SCIP_MAXDEPTHLEVEL:
		return ScipError{"Maximal branching depth level exceeded"};
	default:
		return ScipError{"Invalid return code"};
	}
}

void scip::ScipError::reset_message_capture() {
	ErrorCollector::clear();
}

scip::ScipError::ScipError(std::string message_) : message(std::move(message_)) {}

const char* scip::ScipError::what() const noexcept {
	return message.c_str();
}

/**************************************
 *  Implementation of ErrorCollector  *
 **************************************/

namespace {

thread_local std::string scip::ErrorCollector::errors{};

void scip::ErrorCollector::scip_error(SCIP_MESSAGEHDLR* /*messagehdlr*/, FILE* /*file*/, const char* message) {
	errors += message;
}

void scip::ErrorCollector::clear() {
	errors.clear();
}

std::string scip::ErrorCollector::collect() {
	std::string message{};
	message.reserve(buffer_size);
	std::swap(message, errors);
	return message;
}

scip::ErrorCollector::ErrorCollector() noexcept : ObjMessagehdlr(false) {
	try {
		errors.reserve(buffer_size);
	} catch (std::exception const&) {
		// Cannot reserve space for error string but it can be done (or fail) later
	}
}

void scip::MessageHandlerDeleter::operator()(SCIP_MESSAGEHDLR* ptr) {
	// Cannot use scip::call because it accesses the collector which no longer exists.
	[[maybe_unused]] auto retcode = SCIPmessagehdlrRelease(&ptr);
	assert(retcode == SCIP_OKAY);
}

auto make_unique_hander() {
	SCIP_MESSAGEHDLR* raw_handler = nullptr;
	{
		auto error_collector = std::make_unique<ErrorCollector>();
		// Cannot use scip::call because it accesses the collector which does not exist yet.
		// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks) Give ownership of raw pointer to SCIP
		auto const retcode = SCIPcreateObjMessagehdlr(&raw_handler, error_collector.release(), true);
		assert(raw_handler != nullptr);
		if (retcode != SCIP_OKAY) {
			throw scip::ScipError::from_retcode(retcode);
		}
	}

	decltype(scip::message_handler) unique_handler;
	unique_handler.reset(raw_handler);
	return unique_handler;
}

auto registered_handler() noexcept -> decltype(scip::message_handler) {
	try {
		auto handler = make_unique_hander();
		SCIPsetStaticErrorPrintingMessagehdlr(handler.get());
		return handler;
	} catch (std::exception const& e) {
		std::cerr << "Warning: initialization of SCIP error collector failed with exception\n";
		std::cerr << e.what() << '\n';
		return nullptr;
	}
}

decltype(scip::message_handler) message_handler = registered_handler();

}  // namespace
}  // namespace ecole::scip
