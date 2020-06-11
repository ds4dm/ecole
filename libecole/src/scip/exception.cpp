#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include <objscip/objmessagehdlr.h>

#include "ecole/scip/exception.hpp"

namespace ecole {
namespace scip {

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
	ErrorCollector();

	virtual void scip_error(SCIP_MESSAGEHDLR* messagehdlr, FILE* file, const char* msg) override;
	std::string collect();
	void clear();

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

/**
 * Access the ErrorCollector from the global SCIP_MESSAGEHDLR.
 */
ErrorCollector& get_collector();

}  // namespace

/*****************************
 *  Definition of Exception  *
 *****************************/

Exception Exception::from_retcode(SCIP_RETCODE retcode) {
	auto message = get_collector().collect();
	if (message.size() > 0) {
		return Exception(std::move(message));
	} else {
		switch (retcode) {
		case SCIP_OKAY:
			throw Exception("Normal termination must not raise exception");
		case SCIP_ERROR:
			return Exception("Unspecified error");
		case SCIP_NOMEMORY:
			return Exception("Insufficient memory error");
		case SCIP_READERROR:
			return Exception("File read error");
		case SCIP_WRITEERROR:
			return Exception("File write error");
		case SCIP_BRANCHERROR:
			return Exception("Branch error");
		case SCIP_NOFILE:
			return Exception("File not found error");
		case SCIP_FILECREATEERROR:
			return Exception("Cannot create file");
		case SCIP_LPERROR:
			return Exception("Error in LP solver");
		case SCIP_NOPROBLEM:
			return Exception("No problem exists");
		case SCIP_INVALIDCALL:
			return Exception("Method cannot be called at tException(his time in solution process");
		case SCIP_INVALIDDATA:
			return Exception("Method cannot be called with this type of data");
		case SCIP_INVALIDRESULT:
			return Exception("Method returned an invalid result code");
		case SCIP_PLUGINNOTFOUND:
			return Exception("A required plugin was not found");
		case SCIP_PARAMETERUNKNOWN:
			return Exception("The parameter with the given name was not found");
		case SCIP_PARAMETERWRONGTYPE:
			return Exception("The parameter is not of the expected type");
		case SCIP_PARAMETERWRONGVAL:
			return Exception("The value is invalid for the given parameter");
		case SCIP_KEYALREADYEXISTING:
			return Exception("The given key is already existing in table");
		case SCIP_MAXDEPTHLEVEL:
			return Exception("Maximal branching depth level exceeded");
		default:
			return Exception("Invalid return code");
		}
	}
}

void scip::Exception::reset_message_capture() {
	get_collector().clear();
}

scip::Exception::Exception(std::string const& message_) : message(message_) {}

scip::Exception::Exception(std::string&& message_) : message(std::move(message_)) {}

const char* scip::Exception::what() const noexcept {
	return message.c_str();
}

/**************************************
 *  Implementation of ErrorCollector  *
 **************************************/

namespace {

thread_local std::string scip::ErrorCollector::errors{};

void scip::ErrorCollector::scip_error(SCIP_MESSAGEHDLR*, FILE*, const char* message) {
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

scip::ErrorCollector::ErrorCollector() : ObjMessagehdlr(false) {
	errors.reserve(buffer_size);
}

void scip::MessageHandlerDeleter::operator()(SCIP_MESSAGEHDLR* ptr) {
	// Cannot use scip::call because it accesses the collector which no longer exists.
	auto retcode = SCIPmessagehdlrRelease(&ptr);
	assert(retcode == SCIP_OKAY);
	(void)retcode;
}

auto create_hander() {
	SCIP_MESSAGEHDLR* raw_handler = nullptr;
	// Cannot use scip::call because it accesses the collector which does not exist yet.
	auto retcode = SCIPcreateObjMessagehdlr(&raw_handler, new ErrorCollector{}, true);  // NOLINT
	assert(raw_handler != nullptr);                                                     // NOLINT
	assert(retcode == SCIP_OKAY);
	(void)retcode;

	decltype(scip::message_handler) unique_handler;
	unique_handler.reset(raw_handler);
	return unique_handler;
}

decltype(scip::message_handler) message_handler = [] {
	auto handler = create_hander();
	SCIPsetStaticErrorPrintingMessagehdlr(handler.get());
	return handler;
}();

scip::ErrorCollector& get_collector() {
	return dynamic_cast<scip::ErrorCollector&>(*SCIPgetObjMessagehdlr(message_handler.get()));
}

}  // namespace
}  // namespace scip
}  // namespace ecole
