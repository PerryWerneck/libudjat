

#include "private.h"
#include <udjat/agent.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	recursive_mutex Abstract::Request::Controller::guard;

	Abstract::Request::Controller::Controller() {

		insert("value",[](Abstract::Request &request) {

			find_agent(request.getPath())->get(request);

		});

		insert("state",[](Abstract::Request &request) {

			find_agent(request.getPath())->getState()->get(request);

		});

	}

	Abstract::Request::Controller::~Controller() {
	}

	Abstract::Request::Controller & Abstract::Request::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;
	}

	void insert(const char *name, std::function<void(Abstract::Request &request)> method) {
		Abstract::Request::Controller::getInstance().insert(name,method);
	}

	void Abstract::Request::Controller::insert(const char *name, std::function<void(Abstract::Request &request)> method) {
		lock_guard<recursive_mutex> lock(guard);
		methods.insert(std::make_pair(Atom(name),Method(method)));
	}

	void Abstract::Request::Controller::call(Abstract::Request &request) {

		Atom name(request.name.c_str());
		std::function<void(Abstract::Request &request)> method;

		{
			lock_guard<recursive_mutex> lock(guard);
			auto entry = methods.find(name);
			if(entry == methods.end()) {
				throw system_error(ENOENT,system_category(),"Method search has failed");
			}
			method = entry->second.get();
		}

		// Call method.
		method(request);

	}

}

