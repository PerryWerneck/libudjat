

#include "private.h"

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Abstract::Request::Request(const char *n, const char *p) : name(n), path(p), expiration(0), modification(0) {
	}

	Abstract::Request::Request() : Request("","") {
	}

	Abstract::Request::~Request() {
	}

	void Abstract::Request::setExpirationTimestamp(time_t time) {

		if(expiration) {
			expiration = min(expiration,time);
		} else {
			expiration = time;
		}

	}

	void Abstract::Request::setModificationTimestamp(time_t time) {

		if(modification)
			modification = min(modification,time);
		else
			modification = time;

	}

	Abstract::Request & Abstract::Request::pop(const char *name, int32_t &value) {
		return pop(value);
	}

	Abstract::Request & Abstract::Request::pop(const char *name, uint32_t &value) {
		return pop(value);
	}

	Abstract::Request & Abstract::Request::pop(const char *name, std::string &value) {
		return pop(value);
	}

	Abstract::Request & Abstract::Request::push(const char *name, const int32_t value) {
		return push(value);
	}

	Abstract::Request & Abstract::Request::push(const char *name, const uint32_t value) {
		return push(value);
	}

	Abstract::Request & Abstract::Request::push(const char *name, const char *value) {
		return push(value);
	}

	void Abstract::Request::call() {
		Controller::getInstance().call(*this);
	}

}

