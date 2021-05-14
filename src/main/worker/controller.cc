

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
		cout << "workers\tStarting controller" << endl;
	}

	Worker::Controller::~Controller() {
		if(workers.size()) {
			cerr << "workers\tStopping controller with " << workers.size() << " active worker(s)" << endl;
		} else {
			cout << "workers\tStopping controller" << endl;
		}

	}

	const Worker * Worker::Controller::find(const char *name) const {

		lock_guard<recursive_mutex> lock(guard);

		auto entry = workers.find(name);

		if(entry == workers.end()) {
			clog << "Can't find worker '" << name << "'" << endl;
			throw system_error(ENOENT,system_category(),"Unknown action");
		}

#ifdef DEBUG
		cout << "Found worker '" << entry->second->c_str() << "'" << endl;
#endif // DEBUG

		return entry->second;
	}

	void Worker::Controller::insert(const Worker *worker) {
		lock_guard<recursive_mutex> lock(guard);

		cout << worker->c_str() << "\tInserting worker" << endl;
		workers.insert(make_pair(worker->c_str(),worker));

	}

	void Worker::Controller::remove(const Worker *worker) {
		lock_guard<recursive_mutex> lock(guard);

		auto entry = workers.find(worker->c_str());
		if(entry == workers.end())
			return;

		if(entry->second != worker)
			return;

		cout << worker->c_str() << "\tRemoving worker" << endl;
		workers.erase(entry);

	}

	void Worker::Controller::getInfo(Response &response) {

		Json::Value report(Json::arrayValue);

		for(auto worker : workers) {

			Json::Value value(Json::objectValue);

			value["name"] = worker.second->name.c_str();
			worker.second->info->get(value);

			report.append(value);
		}

		response["workers"] = report;

	}

}

