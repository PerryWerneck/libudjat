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
 #include <private/alert.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/url.h>
 #include <udjat/agent.h>
 #include <iostream>
 #include <udjat/alert/activation.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	mutex Alert::Controller::guard;

	static const Udjat::ModuleInfo moduleinfo{ "Alert controller" };

	Alert::Controller & Alert::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	Alert::Controller::Controller() : Udjat::MainLoop::Service("alerts",moduleinfo), Udjat::Worker("alerts",moduleinfo) {
		cout << "alerts\tInitializing" << endl;
		if(MainLoop::getInstance()) {
			start();
		}
	}

	Alert::Controller::~Controller() {
	}

	void Alert::Controller::remove(const Abstract::Alert *alert) {

		lock_guard<mutex> lock(guard);
		activations.remove_if([alert](auto activation){
			return activation->id == alert;
		});

	}

	void Alert::Controller::push_back(shared_ptr<Udjat::Alert::Activation> activation) {

		if(!activation->options.asyncronous) {
			// It's a syncronous alert, just emit it.
#ifdef DEBUG
			activation->info() << "Running alert in foreground" << endl;
#endif // DEBUG
			activation->run();
			return;
		} else if(!active()) {

			// disabled, run syncronous.
			activation->warning() << "WARNING: The alert controller is disabled, cant retry a failed alert" << endl;
			activation->run();
			return;

		} if(!MainLoop::getInstance()) {

			// No mainloop, run syncronous.
			activation->warning() << "WARNING: The main loop is disabled, cant retry a failed alert" << endl;
			activation->run();
			return;

		}

		// Have mainloop
		{
			lock_guard<mutex> lock(guard);
#ifdef DEBUG
			activation->info() << "Running alert in background" << endl;
#endif // DEBUG
			activations.push_back(activation);
		}

		emit();
	}

	void Alert::Controller::on_timer() {

		ThreadPool::getInstance().push([this](){
			emit();
		});

	}

	void Alert::Controller::reset(time_t seconds) noexcept {

		if(!seconds) {
			seconds = 1;
		}

		// Using threadpool because I cant change a timer from a timer callback.
		ThreadPool::getInstance().push("alert-controller",[this,seconds]{

			MainLoop &mainloop = MainLoop::getInstance();

			if(!mainloop) {

				cerr << "alerts\tUnable to schedule next alert, the mainloop is not active" << endl;

			} else {

				lock_guard<mutex> lock(guard);
				if(activations.empty()) {

#ifdef DEBUG
					cout << "alerts\tPausing controller" << endl;
#endif // DEBUG
					MainLoop::Timer::disable();

				} else {

					MainLoop::Timer::reset(seconds*100);
					MainLoop::Timer::enable();

				}

			}

		});

	}

	void Alert::Controller::emit() noexcept {

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
					ThreadPool::getInstance().push("alert-run",[this,activation]() {

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

	size_t Alert::Controller::running() const noexcept {
		size_t running = 0;
		lock_guard<mutex> lock(guard);
		for(auto activation : activations) {
			if(activation->running()) {
				running++;
			}
		}
		return running;
	}

	void Alert::Controller::clear() noexcept {

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

#ifdef DEBUG
				cout << "agent\t*** Waiting for tasks " << __FILE__ << "(" << __LINE__ << ")" << endl;
#endif // DEBUG
				ThreadPool::getInstance().wait();
#ifdef DEBUG
				cout << "agent\t*** Wait for tasks complete" << endl;
#endif // DEBUG
			}
		}

		// Cleanup active alerts
		{
			// First copy in the reverse order using the mutex
			list<shared_ptr<Udjat::Alert::Activation>> active;
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

	void Alert::Controller::stop() {

		cout << "alerts\tDeactivating controller" << endl;
		clear();

	}

	bool Alert::Controller::get(Request UDJAT_UNUSED(&request), Response &response) const {

		response.reset(Value::Array);

		lock_guard<mutex> lock(guard);
		for(auto activation : activations) {
			activation->getProperties(response.append(Value::Object));
		}

		return true;
	}


 }


