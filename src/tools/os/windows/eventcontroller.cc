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
 #include <udjat/tools/logger.h>
 #include <udjat/win32/exception.h>

 using namespace std;

 namespace Udjat {

	Event::Controller::Controller() {

		Logger::String{"Starting event controller"}.trace("win32");

		if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandlerRoutine,TRUE)==FALSE) {
			Logger::String{
				"Unable to install console handler: ",
				Win32::Exception::format(GetLastError()).c_str()
			}.error("win32");
		} else {
			Logger::String{"Console handler installed"}.trace("win32");
		}
	}

	Event::Controller::~Controller() {

		if(consolehandlertypes.empty()) {
			Logger::String{"Stopping clean event controller"}.trace("win32");
		} else {
			Logger::String{"Stopping event controller with active handlers"}.error("win32");
		}

		if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandlerRoutine,FALSE)==FALSE) {
			Logger::String{"Unable to remove console handler: ",Win32::Exception::format(GetLastError())}.error("win32");
		} else {
			Logger::String{"Console handler removed"}.trace("win32");
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

	BOOL WINAPI Event::Controller::ConsoleHandlerRoutine(DWORD dwCtrlType) {

		/*
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
		*/

	}

 }

