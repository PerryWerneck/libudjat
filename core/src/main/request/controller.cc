

#include "private.h"
#include <iostream>
#include <udjat/agent.h>

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	recursive_mutex Request::Controller::guard;

	Request::Controller::Controller() {

		insert("agent",[](const char *path, Json::Value &value) {

			value = find_agent(path)->as_json();

		});

	}

	Request::Controller::~Controller() {
	}

	Request::Controller & Request::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;
	}

	void Request::insert(const char *name, std::function<void(Request &request)> method) {
		Request::Controller::getInstance().insert(name,method);
	}

	void Request::Controller::insert(const char *name, std::function<void(Request &request)> method) {
		lock_guard<recursive_mutex> lock(guard);
		methods.insert(std::make_pair(Quark(name).c_str(),Method(method)));
	}

	void Request::insert(const char *name, std::function<void(const char *path, Json::Value &value)> method) {
		Request::Controller::getInstance().insert(name,method);
	}

	void Request::Controller::insert(const char *name, std::function<void(const char *path, Json::Value &value)> method) {
		lock_guard<recursive_mutex> lock(guard);
		jmethods.insert(std::make_pair(Quark(name).c_str(),JMethod(method)));
	}

	void Request::Controller::call(Request &request) {

		const std::function<void(Request &request)> method;

		guard.lock();
		auto entry = methods.find(request.name.c_str());
		guard.unlock();

		if(entry == methods.end()) {
			throw system_error(ENOENT,system_category(),"Method search has failed");
		}

		entry->second.call(request);

	}

	void Request::Controller::call(const char *name, const char *path, Json::Value &value) {

		const char * ptr = strrchr(path,'/');
		if(ptr && !(ptr+1))
			throw runtime_error("Object path should not end in \'/\'");

		guard.lock();
		auto entry = jmethods.find(name);
		guard.unlock();

		if(entry == jmethods.end()) {

			// Can't find JSON based method, try request based.

			class Request : public Udjat::Request {
			public:
				Json::Value response;

				Request(const string &name, const string &path) : Udjat::Request(name.c_str(),path.c_str()) {
				}

				Json::Value get() const {
					return response;
				}

				Udjat::Request & push(const char *name, const int32_t value) override {
					response[name] = value;
					return *this;
				}

				Udjat::Request & push(const char *name, const uint32_t value) {
					response[name] = value;
					return *this;
				}

				Udjat::Request & push(const char *name, const char *value) {
					response[name] = value;
					return *this;
				}

			};

			Request r(name,path);
			r.call();
			value = r.get();
			return;

		}

		entry->second.call(path,value);

	}


}

