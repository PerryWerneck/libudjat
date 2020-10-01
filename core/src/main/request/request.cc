

#include "private.h"
#include <cstring>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Request::Request(const char *n, const char *p) : name(n), path(p), expiration(0), modification(0) {
	}

	static const char * getSeparator(const char *path) {

		const char * ptr = strrchr(path,'/');
		if(!ptr)
			throw runtime_error("Object path should be in format /[identifier]/[request]");

		if(!(ptr+1))
			throw runtime_error("Object path should not end in \'/\'");

		return ptr;
	}

	static string getNameFrompath(const char *path) {
		return string(getSeparator(path)+1);
	}

	static string getPathWithoutName(const char *path) {
		return string(path,getSeparator(path)-path);
	}

	Request::Request(const char *path) : Request(getNameFrompath(path).c_str(),getPathWithoutName(path).c_str()) {
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

	Request & Request::pop(const char *name, int32_t &value) {
		return pop(value);
	}

	Request & Request::pop(const char *name, uint32_t &value) {
		return pop(value);
	}

	Request & Request::pop(const char *name, std::string &value) {
		return pop(value);
	}

	Request & Request::push(const char *name, const int32_t value) {
		return push(value);
	}

	Request & Request::push(const char *name, const uint32_t value) {
		return push(value);
	}

	Request & Request::push(const char *name, const char *value) {
		return push(value);
	}

	Request & Request::pop(int32_t &value) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::pop(uint32_t &value) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::pop(std::string &value) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::push(const int32_t value) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::push(const uint32_t value) {
		throw system_error(ENOTSUP,system_category());
	}

	Request & Request::push(const char *value) {
		throw system_error(ENOTSUP,system_category());
	}

	void Request::call() {
		Controller::getInstance().call(*this);
	}

}

