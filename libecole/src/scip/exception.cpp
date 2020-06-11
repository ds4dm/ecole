#include <cstddef>
#include <string>
#include <utility>

#include <objscip/objmessagehdlr.h>

#include "ecole/scip/exception.hpp"

#include "scip/utils.hpp"

namespace ecole {
namespace scip {

/***********************************
 *  Declaration of ErrorCollector  *
 ***********************************/

/**
 * SCIP message handler to collect error messages.
 *
 * This message handler stores error message rather than printing them to standard error.
 * The messages can then be collected by the exception maker to craft the messsage of the exception.
 *
 * The class stores the messages in a thread local variable.
 */
class ErrorCollector : public ::scip::ObjMessagehdlr {
public:
	ErrorCollector();

	virtual void scip_error(SCIP_MESSAGEHDLR* messagehdlr, FILE* file, const char* msg) override;
	std::string clear();

private:
	thread_local static std::string errors;
	constexpr static std::size_t buffer_size = 1000;
};

/**
 * Initialize the handler and register it with SCIP.
 */
namespace {
extern ErrorCollector error_collector;
}

/*****************************
 *  Definition of Exception  *
 *****************************/

Exception Exception::from_retcode(SCIP_RETCODE retcode) {
	auto message = error_collector.clear();
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

Exception::Exception(std::string const& message_) : message(message_) {}

Exception::Exception(std::string&& message_) : message(std::move(message_)) {}

const char* Exception::what() const noexcept {
	return message.c_str();
}

/**************************************
 *  Implementation of ErrorCollector  *
 **************************************/

thread_local std::string ErrorCollector::errors{};

void ErrorCollector::scip_error(SCIP_MESSAGEHDLR*, FILE*, const char* message) {
	errors += message;
}

std::string ErrorCollector::clear() {
	std::string message{};
	message.reserve(buffer_size);
	std::swap(message, errors);
	return message;
}

ErrorCollector::ErrorCollector() : ObjMessagehdlr(false) {
	errors.reserve(buffer_size);
}

namespace {

ErrorCollector error_collector = [] {
	SCIP_MESSAGEHDLR* scip_handler = nullptr;
	ErrorCollector error_handler{};
	scip::call(SCIPcreateObjMessagehdlr, &scip_handler, &error_handler, false);
	SCIPsetStaticErrorPrintingMessagehdlr(scip_handler);
	return error_handler;
}();

}

}  // namespace scip
}  // namespace ecole
