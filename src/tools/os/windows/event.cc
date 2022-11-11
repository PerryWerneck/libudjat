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

 using namespace std;

 namespace Udjat {

	static const struct EventNames {
		DWORD dwCtrlType;
		const char *name;
	} eventnames[] = {
		{ CTRL_C_EVENT,					"Ctrl-C"                                },
		{ CTRL_BREAK_EVENT,				"Ctrl-Break"                    		},
		{ CTRL_CLOSE_EVENT,				"close"                                 },
		{ CTRL_LOGOFF_EVENT,			"logoff"                                },
		{ CTRL_SHUTDOWN_EVENT,			"shutdown"                              },
		{ CTRL_C_EVENT,					"^C"                                    },
		{ CTRL_BREAK_EVENT,				"^Break"                                },
		{ CTRL_C_EVENT,					"CTRL_C_EVENT"                 			},
		{ CTRL_BREAK_EVENT,				"CTRL_BREAK_EVENT"              		},
		{ CTRL_CLOSE_EVENT,				"CTRL_CLOSE_EVENT"              		},
		{ CTRL_LOGOFF_EVENT,			"CTRL_LOGOFF_EVENT"             		},
		{ CTRL_SHUTDOWN_EVENT,			"CTRL_SHUTDOWN_EVENT"   				},
		{ CTRL_C_EVENT,					"SIGTERM"                               },
		{ CTRL_C_EVENT,					"terminate"                             },
		{ CTRL_BREAK_EVENT,				"SIGHUP"  		                  		},
		{ CTRL_BREAK_EVENT,				"reload"  		                  		},
	};

	Event::Controller::ConsoleHandlerType::ConsoleHandlerType(DWORD type) : dwCtrlType(type) {
		cout << "win32\tWatching " << to_string() << endl;
	}

	Event::Controller::ConsoleHandlerType::~ConsoleHandlerType() {
		cout << "win32\tUnwatching " << to_string() << endl;
	}

	const char * Event::Controller::ConsoleHandlerType::to_string() const noexcept {
		for(size_t ix=0;ix < (sizeof(eventnames)/sizeof(eventnames[0]));ix++) {
			if(eventnames[ix].dwCtrlType == dwCtrlType) {
				return eventnames[ix].name;
			}
		}
		return "ConsoleEvent";
	}

	void Event::remove(void *id) {
		Controller::getInstance().remove(id);
	}

	Event & Event::ConsoleHandler(void *id, DWORD dwCtrlType, const std::function<bool()> handler) {
		return Controller::getInstance().ConsoleHandler(id,dwCtrlType,handler);
	}

	/*
	Event & Event::ConsoleHandler(void *id, const char *name, const std::function<bool()> handler) {

		for(size_t ix = 0; ix < (sizeof(eventnames)/sizeof(eventnames[0]));ix++) {
			if(!strcasecmp(eventnames[ix].name,name)){
				return ConsoleHandler(id,eventnames[ix].dwCtrlType,handler);
			}
		}

		throw system_error(EINVAL,system_category(),string{name} + " is not a valid signal name");

	}
	*/

 }
