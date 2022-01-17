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

	Alert::Controller::Controller() {
		Worker::info = &moduleinfo;
		Worker::name = "default";
	}

	Alert::Controller::~Controller() {
		lock_guard<mutex> lock(guard);
		MainLoop::getInstance().remove(this);
	}

	void Alert::Controller::deactivate(Alert *alert) {
		lock_guard<mutex> lock(guard);
		alerts.remove_if([alert](const auto &active){
			if(active.alert != alert)
				return false;
			cout << active.name << "\tDeactivating alert " << active.url << endl;
			return true;
		});
	}

	void Alert::Controller::emit() noexcept {

		ThreadPool::getInstance().push([this]() {

			lock_guard<mutex> lock(guard);
			time_t now = time(0);
			time_t next = now + 600;
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

#ifdef DEBUG
			cout << "alert\tNext check scheduled to " << TimeStamp(next) << endl;
#endif // DEBUG
			{
				MainLoop &mainloop = MainLoop::getInstance();
				if(alerts.empty()) {

					cout << "alerts\tStopping alert controller" << endl;
					mainloop.remove(this);

				} else {

					unsigned long mseconds = max(1UL, (unsigned long) (next - time(0))) * 1000;
					if(!mainloop.reset(this,mseconds)) {
						cout << "alerts\tStarting alert controller" << endl;
						mainloop.insert(this,mseconds,[this]() {
							emit();
							return true;
						});
					}

				}
			}


		});


	}

	void Alert::Controller::activate(Alert *alert) {

		lock_guard<mutex> lock(guard);

		if(!alert->worker) {
			alert->worker = this;
		}

		alert->activations.next = time(0) + alert->timers.start;
		alerts.emplace_back(alert);
		emit();

	}

	void Alert::Controller::activate(Alert *alert, const string &payload) {
		lock_guard<mutex> lock(guard);
		alert->activations.next = time(0) + alert->timers.start;
		alerts.emplace_back(alert,payload);
		emit();
	}


 }
