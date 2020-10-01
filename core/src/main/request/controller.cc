

#include "private.h"
#include <udjat/agent.h>

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
		methods.insert(std::make_pair(Atom(name),Method(method)));
	}

	void Request::Controller::call(Request &request) {

		Atom name(request.name.c_str());
		std::function<void(Request &request)> method;

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

