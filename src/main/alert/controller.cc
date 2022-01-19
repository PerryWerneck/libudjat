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
		cout << "alerts\tInitializing" << endl;

		// Force creation of the default mainloop.
		MainLoop::getInstance();

	}

	Alert::Controller::~Controller() {
		cout << "alerts\tDeinitializing" << endl;
		lock_guard<mutex> lock(guard);
		MainLoop::getInstance().remove(this);
	}

	void Alert::Controller::remove(const Alert *alert) {
		lock_guard<mutex> lock(guard);
		activations.remove_if([alert](const auto &activation){
			if(activation->alert().get() != alert)
				return false;
			return true;
		});
	}

	void Alert::Controller::reset(time_t seconds) noexcept {

		if(!seconds) {
			seconds = 1;
		}

		// Using threadpool because I cant change a timer from a timer callback.
		ThreadPool::getInstance().push([this,seconds]{
#ifdef DEBUG
				cout << "alert\tNext check scheduled to " << TimeStamp(time(0) + seconds) << endl;
#endif // DEBUG

			MainLoop &mainloop = MainLoop::getInstance();

			lock_guard<mutex> lock(guard);
			if(activations.empty()) {

				cout << "alerts\tStopping controller" << endl;
				mainloop.remove(this);

			} else if(!mainloop.reset(this,seconds*1000)) {

				cout << "alerts\tStarting controller" << endl;
				mainloop.insert(this,seconds*1000,[this]() {
					emit();
					return true;
				});

			}

		});

	}

	/*
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
	*/

	/*
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
	*/

	void Alert::Controller::refresh() noexcept {

		time_t now = time(0);
		time_t next = now + 600;

		{
			lock_guard<mutex> lock(guard);
			for(auto activation : activations) {
				if(activation->timers.next > now) {
					next = min(next,activation->timers.next);
				} else {
					next = now+2;
				}
			}
		}

		reset(next-now);

	}

	void Alert::Controller::insert(const std::shared_ptr<Alert::Activation> activation) {

			{
				lock_guard<mutex> lock(guard);
				activations.push_back(activation);
			}

			emit();
	}

	void Alert::Controller::emit() noexcept {

		time_t now = time(0);
		time_t next = now + 600;

		lock_guard<mutex> lock(guard);
		activations.remove_if([this,now,&next](auto activation){

			auto alert = activation->alert();

			if(!activation->timers.next) {
				// No alert or no next, remove from list.
				cout << "alerts\tAlert '" << alert->c_str() << "' was stopped" << endl;
				return true;
			}

			if(activation->timers.next <= now) {

				// Timer has expired
				activation->timers.next = (now + alert->timers.interval);

				if(activation->running) {

					clog << "alerts\tAlert '" << alert->c_str() << "' is running since " << TimeStamp(activation->running) << endl;

				} else {

					activation->running = time(0);

					ThreadPool::getInstance().push([this,activation]() {

						try {
							cout << "alerts\tEmitting '"
								<< activation->name() << "' ("
								<< (activation->count.success + activation->count.failed + 1)
								<< ")"
								<< endl;
							activation->timers.last = time(0);
							activation->emit();
							activation->success();
						} catch(const exception &e) {
							activation->failed();
							cerr << "alerts\tAlert '" << activation->name() << "': " << e.what() << " (" << activation->count.failed << " fail(s))" << endl;
						} catch(...) {
							activation->failed();
							cerr << "alerts\tAlert '" << activation->name() << "' has failed " << activation->count.failed << " time(s)" << endl;
						}
						activation->running = 0;

					});
				}
			}

			next = min(next,activation->timers.next);
			return false;
		});

		reset(next-now);

	}

	bool Alert::Controller::parse(Abstract::State &parent, const pugi::xml_node &node) const {
		parent.append(make_shared<Alert>(node));
		return true;
	}

 }
