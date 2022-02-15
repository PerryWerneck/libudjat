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
		MainLoop::getInstance();
	}

	Abstract::Alert::Controller::~Controller() {
	}

	void Abstract::Alert::Controller::remove(const Abstract::Alert *alert) {

		lock_guard<mutex> lock(guard);
		activations.remove_if([alert](auto activation){
			return activation->alert == alert;
		});

	}

	void Abstract::Alert::Controller::push_back(shared_ptr<Abstract::Alert::Activation> activation) {

		if(!MainLoop::getInstance()) {

			// No mainloop

			if(activation->alert && activation->alert->timers.start) {
				throw runtime_error("Can't emit a delayed alert, the main loop is disabled");
			}

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

		// Using threadpool because I cant change a timer from a timer callback.
		ThreadPool::getInstance().push([this,seconds]{
#ifdef DEBUG
				cout << "alerts\tNext check scheduled to " << TimeStamp(time(0) + seconds) << endl;
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

	void Abstract::Alert::Controller::emit() noexcept {

		time_t now = time(0);
		time_t next = now + 600;

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
				activation->timers.next = (now + activation->timers.interval);

				if(activation->state.running) {

					activation->warning() << "Alert is running since " << TimeStamp(activation->state.running) << endl;
					activation->timers.next = now + max((unsigned int) 5, activation->timers.busy);

				} else {

					activation->state.running = time(0);

					ThreadPool::getInstance().push([this,activation]() {

						activation->run();
						activation->state.running = 0;

					});
				}
			}

			next = min(next,activation->timers.next);
			return false;
		});

		reset(next-now);

	}

	/*



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

		{
			lock_guard<mutex> lock(guard);
			activations.clear();
			MainLoop::getInstance().remove(this);
		}
	}


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


	bool Abstract::Alert::Controller::get(Request UDJAT_UNUSED(&request), Response &response) const {

		response.reset(Value::Array);

		lock_guard<mutex> lock(guard);
		for(auto activation : activations) {
			activation->getProperties(response.append(Value::Object));
		}

		return true;
	}
	*/

 }


