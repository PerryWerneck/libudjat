

#include "private.h"
#include <cstring>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Request::Request(const char *n, const char *p) : name(n), path(p), expiration(0), modification(0) {
	}

	Request::Request() : Request("","") {
	}

	Request::~Request() {
	}

	void Request::setExpirationTimestamp(time_t time) {

		if(expiration) {
			expiration = min(expiration,time);
		} else {
			expiration = time;
		}

	}

	void Request::setModificationTimestamp(time_t time) {

		if(modification)
			modification = min(modification,time);
		else
			modification = time;

	}

	Request & Request::pop(const char UDJAT_UNUSED(*name), int32_t &value) {
		return pop(value);
	}

	Request & Request::pop(const char UDJAT_UNUSED(*name), uint32_t &value) {
		return pop(value);
	}

	Request & Request::pop(const char UDJAT_UNUSED(*name), std::string &value) {
		return pop(value);
	}

	Request & Request::push(const char UDJAT_UNUSED(*name), const int32_t value) {
		return push(value);
	}

	Request & Request::push(const char UDJAT_UNUSED(*name), const uint32_t value) {
		return push(value);
	}

	Request & Request::push(const char UDJAT_UNUSED(*name), const char *value) {
		return push(value);
	}

	Request & Request::pop(int32_t UDJAT_UNUSED(&value)) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::pop(uint32_t UDJAT_UNUSED(&value)) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::pop(std::string UDJAT_UNUSED(&value)) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::push(const int32_t UDJAT_UNUSED(value)) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::push(const uint32_t UDJAT_UNUSED(value)) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::push(const char UDJAT_UNUSED(*value)) {
		throw system_error(ENOTSUP,system_category());
	}

	void Request::call() {
		Controller::getInstance().call(*this);
	}

	void Request::call(const char *cmd, const char *path, Json::Value &value) {
		Controller::getInstance().call(cmd, path,value);
	}

}

