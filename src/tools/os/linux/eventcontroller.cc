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

 #include <private/event.h>
 #include <iostream>
 #include <cstring>
 #include <csignal>
 #include <udjat/tools/threadpool.h>

 using namespace std;

 namespace Udjat {

	Event::Controller::Signal & Event::Controller::SignalFactory(int signum) {

		for(Signal &signal : signals) {
			if(signal.signum == signum) {
				return signal;
			}
		}

		// Not found, create new signal.
		signals.emplace_front(signum);
		return signals.front();
	}

	Event & Event::Controller::SignalHandler(void *id, int signum, const std::function<bool()> handler) {
		lock_guard<mutex> lock(guard);

		Signal &signal = SignalFactory(signum);
		signal.insert(id,handler);

		return signal;
	}

	void Event::Controller::remove(void *id) {
		lock_guard<mutex> lock(guard);

		signals.remove_if([id](Signal &signal){
			signal.listeners.remove_if([id](Signal::Listener &listener){
				return listener.id == id;
			});
			return signal.empty();
		});

	}

	void Event::Controller::onSignal(int signum) noexcept {
		cout << "signals\tProcessing signal '" << strsignal(signum) << "' (" << signum << ")" << endl;

		Controller &instance = getInstance();

		{
			lock_guard<mutex> lock(guard);
			for(Signal &signal : instance.signals) {
				if(signal.signum == signum) {
					ThreadPool::getInstance().push([&signal]() {
						signal.trigger();
					});
				}
			}
		}

	}

 }

