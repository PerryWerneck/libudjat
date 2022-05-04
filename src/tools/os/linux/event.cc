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
 #include <udjat/defs.h>
 #include <udjat/tools/event.h>
 #include <iostream>
 #include <vector>
 #include <csignal>
 #include <cstring>
 #include <mutex>
 #include <udjat/tools/threadpool.h>

 using namespace std;

 namespace Udjat {

	static void onSignal(int signum) noexcept;

 	class SignalEvent : public Event {
	public:
		int signum;

		SignalEvent(int s) : signum(s) {
			signal(signum,onSignal);
			cout << "signals\tWatching '" << strsignal(signum) << "'" << endl;
		}

		virtual ~SignalEvent() {
			cout << "signals\tUnwatching '" << strsignal(signum) << "'" << endl;
			signal(signum,SIG_DFL);
		}

		std::string to_string() const noexcept override {
			return strsignal(signum);
		}

	};

	class Controller : public vector<SignalEvent> {
	private:
		Controller() {
			cout << "signals\tInitializing controller" << endl;
		}

	public:
		Controller(const Controller &src) = delete;
		Controller(const Controller *src) = delete;

		static Controller & getInstance() {
			static Controller instance;
			return instance;
		}
	};

	Event & Event::SignalEventFactory(int signum) {

		Controller &controller = Controller::getInstance();

		for(SignalEvent &event : controller) {
			if(event.signum == signum) {
				return event;
			}
		}

		// Not found, create a new one.
		controller.emplace_back(signum);

		return controller.back();
	}

	void onSignal(int signum) noexcept {
		cout << "signals\tReceived '" << strsignal(signum) << "'" << endl;

		ThreadPool::getInstance().push([signum]() {
			for(SignalEvent &event : Controller::getInstance()) {
				if(event.signum == signum) {
					event.trigger();
					break;
				}
			}
		});

	}

 }
