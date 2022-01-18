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

 #include "private.h"
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/state.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	mutex Alert::Controller::guard;

	static const Udjat::ModuleInfo moduleinfo {
		PACKAGE_NAME,									// The module name.
		"Alert controller",			 					// The module description.
		PACKAGE_VERSION, 								// The module version.
		PACKAGE_URL, 									// The package URL.
		PACKAGE_BUGREPORT 								// The bugreport address.
	};

	Alert::Controller & Alert::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	Alert::Controller::Controller() : Udjat::Factory("alert",&moduleinfo) {
		Worker::info = &moduleinfo;
		Worker::name = "default";
		cout << "alerts\tInitializing" << endl;

		// Force creation of the default mainloop.
		MainLoop::getInstance();

	}

	Alert::Controller::~Controller() {
		cout << "alerts\tDeinitializing" << endl;
		lock_guard<mutex> lock(guard);
		MainLoop::getInstance().remove(this);
	}

	void Alert::Controller::remove(std::shared_ptr<Alert> alert) {
		lock_guard<mutex> lock(guard);
		alerts.remove_if([alert](const auto &active){
			if(active.alert.get() != alert.get())
				return false;
			cout << active.name() << "\tDeactivating " << active.url << endl;
			return true;
		});
	}

	void Alert::Controller::reset(time_t seconds) noexcept {

		if(!seconds) {
			seconds = 1;
		}

#ifdef DEBUG
			cout << "alert\tNext check scheduled to " << TimeStamp(time(0) + seconds) << endl;
#endif // DEBUG

		MainLoop &mainloop = MainLoop::getInstance();

		lock_guard<mutex> lock(guard);
		if(alerts.empty()) {

			cout << "alerts\tStopping alert controller" << endl;
			mainloop.remove(this);

		} else {

			if(!mainloop.reset(this,seconds*1000)) {
				cout << "alerts\tStarting alert controller" << endl;
				mainloop.insert(this,seconds*1000,[this]() {
					emit();
					return true;
				});
			}

		}

	}

	void Alert::Controller::insert(const Alert::Worker *worker) {
		lock_guard<mutex> lock(guard);
		workers.push_back(worker);
	}

	void Alert::Controller::remove(const Alert::Worker *worker) {
		lock_guard<mutex> lock(guard);
		workers.remove_if([worker](const Worker *wrk){
			return wrk == worker;
		});
	}

	const Alert::Worker * Alert::Controller::getWorker(const char *name) const {

		if(!(name && *name) || strcasecmp(name,"default") == 0) {
			return this;
		}

		for(auto worker : workers) {
			if(!strcasecmp(worker->name,name)) {
				return worker;
			}
		}

		cerr << "alert\tCan't find alert engine '" << name << "' using the default one" << endl;
		return this;
	}


	void Alert::Controller::refresh() noexcept {

		time_t now = time(0);
		time_t next = now + 600;

		{
			lock_guard<mutex> lock(guard);
			for(auto active = alerts.begin(); active != alerts.end(); active++) {

				if(active->alert->activations.next > now) {
					next = min(next,active->alert->activations.next);
				} else {
					next = now+2;
				}

			}
		}

		reset(next-now);

	}

	void Alert::Controller::emit() noexcept {

		ThreadPool::getInstance().push([this]() {

			time_t now = time(0);
			time_t next = now + 600;
			{
				lock_guard<mutex> lock(guard);
				alerts.remove_if([now,&next](const auto &active){

					if(!(active.alert && active.alert->activations.next))
						// No alert or no next, remove from list.
						return true;

					if(active.alert->activations.next <= now) {
						// Timer has expired, emit action.
						active.alert->emit(active);
					}

					next = min(next,active.alert->activations.next);
					return false;
				});
			}

			reset(next-now);

		});


	}

	void Alert::Controller::insert(std::shared_ptr<Alert> alert, const std::string &url, const std::string &payload)  {
		lock_guard<mutex> lock(guard);

		if(!alert->worker) {
			alert->worker = this;
		}

		alert->activations.next = time(0) + alert->timers.start;
		alerts.emplace_back(alert,url,payload);
		emit();
	}

	bool Alert::Controller::parse(Abstract::State &parent, const pugi::xml_node &node) const {
		parent.append(make_shared<Alert>(node));
		return true;
	}


 }
