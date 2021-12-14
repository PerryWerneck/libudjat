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
 #include <csignal>
 #include <cstring>
 #include <iostream>

 namespace Udjat {

	class ServiceController : public MainLoop {
	private:
		static BOOL WINAPI ConsoleHandler(DWORD event);

	public:
		ServiceController();
		~ServiceController();

		static ServiceController & getInstance();

	};

	ServiceController & ServiceController::getInstance() {
		static ServiceController instance;
		return instance;
	}

 	MainLoop & MainLoop::getInstance() {
 		return ServiceController::getInstance();
	}

	ServiceController::ServiceController() : MainLoop() {
		if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandler,TRUE)==FALSE) {
			cerr << "mainloop\tUnable to install console handler" << endl;
		}
	}

	ServiceController::~ServiceController() {
		if (SetConsoleCtrlHandler( (PHANDLER_ROUTINE)ConsoleHandler,FALSE)==FALSE) {
			cerr << "mainloop\tUnable to remove console handler" << endl;
		}
	}

	BOOL WINAPI ServiceController::ConsoleHandler(DWORD event) {

		ServiceController &controller = ServiceController::getInstance();

		switch(event) {
		case CTRL_C_EVENT:
			cerr << "mainloop\tCTRL+C received!" << endl;
			controller.enabled = false;
			break;

		case CTRL_BREAK_EVENT:
		   cerr << "mainloop\tCTRL+BREAK received!" << endl;
			controller.enabled = false;
			break;

		case CTRL_CLOSE_EVENT:
			cerr << "mainloop\tProgram being closed!" << endl;
			controller.enabled = false;
			break;

		case CTRL_LOGOFF_EVENT:
			cerr << "mainloop\tUser is logging off!" << endl;
			controller.enabled = false;
			break;

		case CTRL_SHUTDOWN_EVENT:
			cerr << "mainloop\tUser is logging off!" << endl;
			controller.enabled = false;
			break;

		}

		controller.wakeup();

		return TRUE;

	}

 }
