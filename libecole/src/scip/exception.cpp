#include <cstddef>
#include <mutex>
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
 * The messages can then me collected by the exception maker to craft the messsage of the exception.
 *
 * Given that the SCIP error message handler is global, so is this message handler.
 * It implements the singleton pattern that lazily created and register with SCIP on first access.
 * However, because it needs to be registered with SCIP before the first exception is thrown,
 * a function in this file for instanciation when loading the library.
 * Using the message handler thread safe.
 */
class ErrorCollector : public ::scip::ObjMessagehdlr {
public:
	static ErrorCollector& get_handler();

	ErrorCollector(ErrorCollector const&) = delete;
	ErrorCollector(ErrorCollector&&) = delete;
	ErrorCollector& operator=(ErrorCollector const&) = delete;
	ErrorCollector& operator=(ErrorCollector&&) = delete;

	virtual void scip_error(SCIP_MESSAGEHDLR* messagehdlr, FILE* file, const char* msg) override;
	std::string clear();

private:
	static constexpr std::size_t buffer_size = 1000;
	std::mutex mut;
	std::string errors;

	ErrorCollector();
};

/*****************************
 *  Definition of Exception  *
 *****************************/

Exception Exception::from_retcode(SCIP_RETCODE retcode) {
	auto message = ErrorCollector::get_handler().clear();
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

ErrorCollector& ErrorCollector::get_handler() {
	// Guaranteed to be destroyed and instantiated on first use.
	static ErrorCollector handler{};
	return handler;
}

void ErrorCollector::scip_error(SCIP_MESSAGEHDLR*, FILE*, const char* message) {
	std::lock_guard<std::mutex> lk(mut);
	errors += message;
}

std::string ErrorCollector::clear() {
	std::string message{};
	message.reserve(buffer_size);
	std::lock_guard<std::mutex> lk(mut);
	std::swap(message, errors);
	return message;
}

ErrorCollector::ErrorCollector() : ObjMessagehdlr(false) {
	SCIP_MESSAGEHDLR* scip_handler = nullptr;
	scip::call(SCIPcreateObjMessagehdlr, &scip_handler, this, false);
	SCIPsetStaticErrorPrintingMessagehdlr(scip_handler);
	errors.reserve(buffer_size);
}

/**
 * A dummy global varaible to force registering the handler with SCIP at laoding time.
 */
static auto const force = [] {
	ErrorCollector::get_handler();
	return 0;
}();

}  // namespace scip
}  // namespace ecole
