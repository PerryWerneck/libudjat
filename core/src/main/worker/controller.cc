

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

	const Worker * Worker::Controller::find(const char *name) const {

		lock_guard<recursive_mutex> lock(guard);
		auto entry = workers.find(name);

		if(entry == workers.end()) {
			throw system_error(ENOENT,system_category(),"Can't find requested action");
		}

		if(!entry->second->active) {
			throw system_error(ENOENT,system_category(),"Requested action is not available");
		}

		return entry->second;
	}

	void Worker::Controller::insert(const Worker *worker) {
		lock_guard<recursive_mutex> lock(guard);

		workers.insert(make_pair(worker->c_str(),worker));
	}

	void Worker::Controller::remove(const Worker *worker) {
		lock_guard<recursive_mutex> lock(guard);

		auto entry = workers.find(worker->c_str());
		if(entry == workers.end())
			return;

		if(entry->second != worker)
			return;

		workers.erase(entry);

	}

}

