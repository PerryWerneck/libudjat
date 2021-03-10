

#include "private.h"

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	recursive_mutex Worker::Controller::guard;

	Worker::Controller & Worker::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;
	}

	Worker::Controller::Controller() {


	}

	Worker::Controller::~Controller() {

	}

	void Worker::Controller::insert(Worker *worker) {
		lock_guard<recursive_mutex> lock(guard);


	}

	void Worker::Controller::remove(Worker *worker) {
		lock_guard<recursive_mutex> lock(guard);

	}

	/*

	Request::Controller::Controller() {

		insert(I_("agent"),[](const char *path, const Json::Value &request, Json::Value &response) {

			response = find_agent(path)->as_json();

		});

		insert(I_("states"),[](const char *path, const Json::Value &request, Json::Value &response) {

			response = Json::Value(Json::arrayValue);

			find_agent(path)->foreach([&response](Abstract::Agent &agent){

				if(!agent.hasOwnStates())
					return;

				Json::Value row;
				row["name"] = agent.getName();
				row["icon"] = agent.getIcon().c_str();
				row["label"] = agent.getLabel().c_str();

				auto state = agent.getState();
				row["summary"] = state->getSummary().c_str();
				row["body"] = state->getBody().c_str();
				state->getLevel(row);

				auto activation_time = state->getActivationTime();

				if(activation_time)
					row["timestamp"] = TimeStamp(activation_time).to_string(TIMESTAMP_FORMAT_JSON);
				else
					row["timestamp"] = 0;

				response.append(row);
			});


		});

	}

	Request::Controller::~Controller() {
	}


	void Request::insert(const Quark &name, std::function<void(Request &request)> method) {
		Request::Controller::getInstance().insert(name,method);
	}

	void Request::Controller::insert(const Quark &name, std::function<void(Request &request)> method) {
		lock_guard<recursive_mutex> lock(guard);
		methods.insert(std::make_pair(name.c_str(),Method(method)));
	}

	void Request::insert(const Quark &name, std::function<void(const char *path, const Json::Value &request, Json::Value &response)> method) {
		Request::Controller::getInstance().insert(name,method);
	}

	void Request::Controller::insert(const Quark &name, std::function<void(const char *path, const Json::Value &request, Json::Value &response)> method) {
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

	void Request::Controller::call(const char *name, const char *path, const Json::Value &request, Json::Value &response) {

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
			response = r.get();
			return;

		}

		entry->second.call(path,request,response);

	}
	*/

}

