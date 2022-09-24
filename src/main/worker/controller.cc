/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <private/worker.h>
 #include <udjat/moduleinfo.h>

 using namespace std;

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
			cout << "workers\tStopping clean controller" << endl;
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

		cout << "workers\tRegister '" << worker->name << "' (" << worker->module.description << ") " << endl;
		workers.insert(make_pair(worker->c_str(),worker));

	}

	void Worker::Controller::remove(const Worker *worker) {
		lock_guard<recursive_mutex> lock(guard);

		auto entry = workers.find(worker->c_str());
		if(entry == workers.end())
			return;

		if(entry->second != worker)
			return;

		cout << "workers\tUnregister '" << worker->name << "' (" << worker->module.description << ") " << endl;
		workers.erase(entry);

	}

	void Worker::Controller::getInfo(Response &response) noexcept {

		response.reset(Value::Array);

		for(auto worker : workers) {

			Value &object = response.append(Value::Object);

			object["name"] = worker.second->name;
			worker.second->module.get(object);

		}

	}

 }

