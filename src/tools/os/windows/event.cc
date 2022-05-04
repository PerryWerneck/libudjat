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
 #include <forward_list>
 #include <csignal>
 #include <cstring>
 #include <mutex>
 #include <udjat/tools/threadpool.h>

 using namespace std;

 namespace Udjat {

	static const struct EventNames {
		DWORD dwCtrlType;
		const char *name;
	} eventnames[] = {
		{ CTRL_C_EVENT,			"Ctrl-C"				},
		{ CTRL_BREAK_EVENT,		"Ctrl-Break"			},
		{ CTRL_CLOSE_EVENT,		"close"					},
		{ CTRL_LOGOFF_EVENT,	"logoff"				},
		{ CTRL_SHUTDOWN_EVENT,	"shutdown"				},
		{ CTRL_C_EVENT,			"^C"					},
		{ CTRL_BREAK_EVENT,		"^Break"				},
		{ CTRL_C_EVENT,			"CTRL_C_EVENT"			},
		{ CTRL_BREAK_EVENT,		"CTRL_BREAK_EVENT"		},
		{ CTRL_CLOSE_EVENT,		"CTRL_CLOSE_EVENT"		},
		{ CTRL_LOGOFF_EVENT,	"CTRL_LOGOFF_EVENT"		},
		{ CTRL_SHUTDOWN_EVENT,	"CTRL_SHUTDOWN_EVENT"	},
	};

 	class ConsoleEvent : public Event {
	public:
		DWORD dwCtrlType;

		ConsoleEvent(DWORD t) : dwCtrlType(t) {
			cout << "win32\tWatching '" << to_string() << "'" << endl;
		}

		virtual ~ConsoleEvent() {
			cout << "win32\tUnwatching '" << to_string() << "'" << endl;
		}

		std::string to_string() const noexcept override {
			for(size_t ix=0;ix < (sizeof(eventnames)/sizeof(eventnames[0]));ix++) {
				if(eventnames[ix].dwCtrlType == dwCtrlType) {
					return eventnames[ix].name;
				}
			}
			return "ConsoleEvent";
		}

		void remove(void *id) override;

	};

	class Controller : public forward_list<ConsoleEvent> {
	private:

		static BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType);

		Controller() {
			cout << "win32\tInitializing console handler" << endl;
			if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandlerRoutine,TRUE)==FALSE) {
				cerr << "win32\tUnable to install console handler" << endl;
			} else {
				cout << "win32\tConsole handler installed" << endl;
			}
		}

		~Controller() {
			if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandlerRoutine,FALSE)==FALSE) {
				cerr << "win32\tUnable to remove console handler" << endl;
			} else {
				cout << "win32\tConsole handler removed" << endl;
			}
		}

	public:

		Controller(const Controller &src) = delete;
		Controller(const Controller *src) = delete;

		static Controller & getInstance();

	};

	Controller & Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	BOOL WINAPI Controller::ConsoleHandlerRoutine(DWORD dwCtrlType) {

		ThreadPool::getInstance().push([dwCtrlType]() {
			for(ConsoleEvent &event : Controller::getInstance()) {
				if(event.dwCtrlType == dwCtrlType) {
					cout << "win32\tHandling '" << event.to_string() << "'" << endl;
					event.trigger();
					break;
				}
			}
		});

		return TRUE;

	}

	void ConsoleEvent::remove(void *id) {

		Event::remove(id);

		if(empty()) {
			Controller::getInstance().remove_if([this](ConsoleEvent &event){
				return &event == this;
			});
		}

	}

	Event & Event::ConsoleHandlerFactory(DWORD dwCtrlType) {

		Controller &controller = Controller::getInstance();

		for(ConsoleEvent &event : controller) {
			if(event.dwCtrlType == dwCtrlType) {
				return event;
			}
		}

		// Not found, create a new one.
		controller.emplace_front(dwCtrlType);

		return controller.front();

	}

 }
