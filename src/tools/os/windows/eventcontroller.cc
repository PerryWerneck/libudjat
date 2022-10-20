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
 #include <private/mainloop.h>
 #include <udjat/tools/logger.h>
 #include <udjat/win32/exception.h>

 using namespace std;

 namespace Udjat {

	Event::Controller::Controller() {
		cout << "win32\tStarting event controller " << hex << this << dec << endl;
		if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandlerRoutine,TRUE)==FALSE) {
			cerr << "win32\tUnable to install console handler" << endl;
		} else {
			cout << "win32\tConsole handler installed" << endl;
		}
	}

	Event::Controller::~Controller() {
		if(consolehandlertypes.empty()) {
			cout << "win32\tStopping clean event controller " << hex << this << dec << endl;
		} else {
			cout << "win32\tStopping event controller " << hex << this << dec << " with active handlers" << endl;
		}
		if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandlerRoutine,FALSE)==FALSE) {
			cerr << "win32\tUnable to remove console handler" << endl;
		} else {
			cout << "win32\tConsole handler removed" << endl;
		}
	}

	Event::Controller::ConsoleHandlerType & Event::Controller::ConsoleHandlerTypeFactory(DWORD dwCtrlType) {

		for(ConsoleHandlerType &type : consolehandlertypes) {
			if(type.dwCtrlType == dwCtrlType) {
				return type;
			}
		}

		// Not found, create new signal.
		consolehandlertypes.emplace_front(dwCtrlType);
		return consolehandlertypes.front();
	}

	Event & Event::Controller::ConsoleHandler(void *id, DWORD dwCtrlType, const std::function<bool()> handler) {
		lock_guard<recursive_mutex> lock(guard);

		ConsoleHandlerType &type = ConsoleHandlerTypeFactory(dwCtrlType);
		type.insert(id,handler);

		return type;
	}

	void Event::Controller::remove(void *id) {
		lock_guard<recursive_mutex> lock(guard);

		consolehandlertypes.remove_if([id](ConsoleHandlerType &type){
			type.listeners.remove_if([id](Event::Listener &listener){
				return listener.id == id;
			});
			return type.empty();
		});

	}

	/*
	bool Event::Controller::call(DWORD dwCtrlType) noexcept {

		debug("Will lock");
		lock_guard<mutex> lock(guard);
		debug("Locked");

		for(ConsoleHandlerType &type : consolehandlertypes) {
			if(type.dwCtrlType == dwCtrlType) {
				debug("Pushing event controller task");
				ThreadPool::getInstance().push("EventController",[&type]() {
					debug("Running trigger");
					type.trigger();
					debug("Trigger complete");
				});
				return true;
			}
		}
		return false;
	}
	*/

	BOOL WINAPI Event::Controller::ConsoleHandlerRoutine(DWORD dwCtrlType) {

		BOOL rc = FALSE;

		switch(dwCtrlType) {
		case CTRL_C_EVENT:
			clog << "win32\tCTRL+C received!" << endl;
			break;

		case CTRL_BREAK_EVENT:
			clog << "win32\tCTRL+BREAK received!" << endl;
			break;

		case CTRL_CLOSE_EVENT:
			clog << "win32\tProgram being closed!" << endl;
			break;

		case CTRL_LOGOFF_EVENT:
			clog << "win32\tUser is logging off!" << endl;
			break;

		case CTRL_SHUTDOWN_EVENT:
			clog << "win32\tSystem is shutting down!" << endl;
			break;

		}

		{
			lock_guard<recursive_mutex> lock(guard);

			for(ConsoleHandlerType &type : Controller::getInstance().consolehandlertypes) {
				if(type.dwCtrlType == dwCtrlType) {
					debug("Pushing console handler");
					ThreadPool::getInstance().push("ConsoleHandler",[&type]() {
						type.trigger();
					});
					rc = TRUE;
				}
			}

		}

		return rc;
	}

 }

