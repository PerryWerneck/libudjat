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
 #include <udjat/module/info.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	recursive_mutex Worker::Controller::guard;

	Worker::Controller & Worker::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller controller;
		return controller;
	}

	Worker::Controller::Controller() {
		Logger::String{
			"Starting controller"
		}.trace("workers");
	}

	Worker::Controller::~Controller() {
		if(workers.size()) {
			cerr << "workers\tStopping controller with " << workers.size() << " active worker(s)" << endl;
		} else {
			Logger::String{"Stopping clean controller"}.trace("workers");
		}

	}

	void Worker::Controller::insert(const Worker *worker) {
		lock_guard<recursive_mutex> lock(guard);
		Logger::String("Adding '",worker->name,"' (",worker->module.description,")").trace("workers");
		workers.push_back(worker);
	}

	void Worker::Controller::remove(const Worker *worker) {
		lock_guard<recursive_mutex> lock(guard);
		Logger::String("Removing '",worker->name,"' (",worker->module.description,")").trace("workers");
		workers.remove(worker);
	}

	bool Worker::Controller::for_each(const std::function<bool(const Worker &worker)> &func) {

		lock_guard<recursive_mutex> lock(guard);
		for(const Worker * worker : workers) {
			if(func(*worker)) {
				return true;
			}
		}
		return false;

	}

 }

