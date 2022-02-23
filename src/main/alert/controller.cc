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
 #include <udjat/tools/url.h>
 #include <udjat/agent.h>
 #include <udjat/state.h>
 #include <iostream>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	mutex Abstract::Alert::Controller::guard;

	static const Udjat::ModuleInfo moduleinfo{ "Alert controller" };

	Abstract::Alert::Controller & Abstract::Alert::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	Abstract::Alert::Controller::Controller() : Udjat::MainLoop::Service("alerts",moduleinfo), Udjat::Worker("alerts",moduleinfo) {
		cout << "alerts\tInitializing" << endl;
		if(MainLoop::getInstance()) {
			start();
		}
	}

	Abstract::Alert::Controller::~Controller() {
	}

	void Abstract::Alert::Controller::remove(const Abstract::Alert *alert) {

		lock_guard<mutex> lock(guard);
		activations.remove_if([alert](auto activation){
			return activation->id == alert;
		});

	}

	void Abstract::Alert::Controller::push_back(shared_ptr<Abstract::Alert::Activation> activation) {

		if(!MainLoop::getInstance()) {

			// No mainloop
			activation->warning() << "WARNING: The main loop is disabled, cant retry a failed alert" << endl;
			activation->run();
			return;

		}

		// Have mainloop
		{
			lock_guard<mutex> lock(guard);
			activations.push_back(activation);
		}

		emit();
	}

	void Abstract::Alert::Controller::reset(time_t seconds) noexcept {

		if(!seconds) {
			seconds = 1;
		}

#ifdef DEBUG
		cout <<  __FILE__ << "(" << __LINE__ << ") seconds=" << seconds << endl;
#endif //

		// Using threadpool because I cant change a timer from a timer callback.
		ThreadPool::getInstance().push([this,seconds]{
#ifdef DEBUG
			cout << "alerts\tNext check scheduled to " << TimeStamp(time(0) + seconds) << endl;
#endif // DEBUG

			MainLoop &mainloop = MainLoop::getInstance();

			if(!mainloop) {

				cerr << "alerts\tUnable to schedule next alert, the mainloop is not active" << endl;

			} else {

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

			}

		});

	}

	void Abstract::Alert::Controller::emit() noexcept {

		time_t now = time(0);
		time_t next = 0;

		lock_guard<mutex> lock(guard);
		activations.remove_if([this,now,&next](auto activation){

			if(!activation->timers.next) {
				// No alert or no next, remove from list.
				if(activation->verbose()) {
					activation->info() << "Alert was stopped" << endl;
				}
				return true;
			}

			if(activation->timers.next <= now) {

				// Timer has expired
				if(activation->state.running) {

					activation->warning() << "Alert is running since " << TimeStamp(activation->state.running) << endl;
					activation->timers.next = now + max((unsigned int) 5, activation->timers.busy);

				} else {

					if(activation->timers.interval) {
						activation->timers.next = (now + activation->timers.interval);
					} else {
						activation->timers.next = now + 60;
					}

					activation->state.running = time(0);
					ThreadPool::getInstance().push([this,activation]() {

						activation->run();
						activation->state.running = 0;

					});
				}
			}

			if(activation->timers.next) {
				if(next) {
					next = min(next,activation->timers.next);
				} else {
					next = activation->timers.next;
				}
			}

			return false;
		});

		if(next) {
			reset(next-now);
		} else {
			reset(5);
		}

	}

	size_t Abstract::Alert::Controller::running() const noexcept {
		size_t running = 0;
		lock_guard<mutex> lock(guard);
		for(auto activation : activations) {
			if(activation->running()) {
				running++;
			}
		}
		return running;
	}

	void Abstract::Alert::Controller::stop() {

		cout << "alerts\tDeactivating controller" << endl;
		MainLoop::getInstance().remove(this);

		{
			size_t pending_activations = running();
			if(pending_activations) {
				clog << "alerts\tWaiting for " << pending_activations << " activations to complete" << endl;
				for(size_t timer = 0; timer < 100 && running(); timer++) {
#ifdef _WIN32
					Sleep(100);
#else
					usleep(100);
#endif // _WIN32
				}
			}
			pending_activations = running();
			if(pending_activations) {
				clog << "alerts\tStopping with " << pending_activations << " activations still active" << endl;
				ThreadPool::getInstance().wait();
			}
		}

		// Cleanup active alerts
		{
			// First copy in the reverse order using the mutex
			list<shared_ptr<Abstract::Alert::Activation>> active;
			{
				lock_guard<mutex> lock(guard);
				for(auto activation : activations) {
					active.push_front(activation);
				}

				// Clear activations list.
				activations.clear();
			}

			// Second, notify and remove.
			active.remove_if([](auto activation){
				activation->warning() << "Cancelling active alert" << endl;
				return true;
			});
		}

	}

	bool Abstract::Alert::Controller::get(Request UDJAT_UNUSED(&request), Response &response) const {

		response.reset(Value::Array);

		lock_guard<mutex> lock(guard);
		for(auto activation : activations) {
			activation->getProperties(response.append(Value::Object));
		}

		return true;
	}

	/*





	void Abstract::Alert::Controller::refresh() noexcept {

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


	*/

 }


