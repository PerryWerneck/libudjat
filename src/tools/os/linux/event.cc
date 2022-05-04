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
 #include <private/event.h>
 #include <iostream>
 #include <forward_list>
 #include <csignal>
 #include <cstring>
 #include <mutex>
 #include <csignal>
 #include <cstring>

 using namespace std;

 namespace Udjat {

	static const struct EventNames {
		int signum;
		const char *name;
	} eventnames[] = {
		{ SIGTERM,						"Ctrl-C"                                },
		{ SIGTERM,						"CTRL_C_EVENT"                 			},
		{ SIGTERM,						"terminate"                 			},
		{ SIGHUP,						"reload" 	                			},
	};

	Event::Controller::Signal::Signal(int s) : signum(s) {
		cout << "signal\tWatching " << strsignal(signum) << endl;
		signal(signum,Controller::onSignal);
	}

	Event::Controller::Signal::~Signal() {
		cout << "signal\tUnwatching " << strsignal(signum) << endl;
		signal(signum,SIG_DFL);
	}

	const char * Event::Controller::Signal::to_string() const noexcept {
		return strsignal(signum);
	}

	void Event::remove(void *id) {
		Controller::getInstance().remove(id);
	}

	Event & Event::SignalHandler(void *id, int signum, const std::function<bool()> handler) {
		return Controller::getInstance().SignalHandler(id,signum,handler);
	}

	Event & Event::SignalHandler(void *id, const char *name, const std::function<bool()> handler) {

		for(size_t ix = 0; ix < (sizeof(sys_siglist)/sizeof(sys_siglist[0]));ix++) {
			if(!strcasecmp(sys_siglist[ix],name)){
				return SignalHandler(id,ix,handler);
			}
		}

		for(size_t ix = 0; ix < (sizeof(eventnames)/sizeof(eventnames[0]));ix++) {
			if(!strcasecmp(eventnames[ix].name,name)){
				return SignalHandler(id,eventnames[ix].signum,handler);
			}
		}

		throw system_error(EINVAL,system_category(),string{name} + " is not a valid signal name");

	}

 }
