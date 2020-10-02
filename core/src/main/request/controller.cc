

#include "private.h"
#include <iostream>
#include <udjat/agent.h>

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	recursive_mutex Request::Controller::guard;

	Request::Controller::Controller() {

		insert("value",[](Request &request) {

			find_agent(request.getPath())->get(request);

		});

		insert("state",[](Request &request) {

			auto agent = find_agent(request.getPath());

			agent->getState()->get(request);
			agent->get("value",request);

		});

		insert("detailed",[](const char *path, Json::Value &value) {

			value = find_agent(path)->as_json();

		});

	}

	Request::Controller::~Controller() {
	}

	static const char * getSeparator(const char *path) {

		const char * ptr = strrchr(path,'/');
		if(!ptr)
			throw runtime_error("Object path should be in format /[identifier]/[command]");

		if(!(ptr+1))
			throw runtime_error("Object path should not end in \'/\'");

		return ptr;
	}

	string Request::Controller::getNameFrompath(const char *path) {
		return string(getSeparator(path)+1);
	}

	string Request::Controller::getPathWithoutName(const char *path) {
		return string(path,getSeparator(path)-path);
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
		methods.insert(std::make_pair(Atom(name).c_str(),Method(method)));
	}

	void Request::insert(const char *name, std::function<void(const char *path, Json::Value &value)> method) {
		Request::Controller::getInstance().insert(name,method);
	}

	void Request::Controller::insert(const char *name, std::function<void(const char *path, Json::Value &value)> method) {
		lock_guard<recursive_mutex> lock(guard);
		jmethods.insert(std::make_pair(Atom(name).c_str(),JMethod(method)));
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

	void Request::Controller::call(const char *p, Json::Value &value) {

		string name = getNameFrompath(p);
		string path = getPathWithoutName(p);

		guard.lock();
		auto entry = jmethods.find(name.c_str());
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

		entry->second.call(path.c_str(),value);

	}


}

