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
 #include <udjat/win32/service.h>
 #include <udjat/win32/exception.h>
 #include <iostream>
 #include <udjat/tools/application.h>

 using namespace std;

 namespace Udjat {

	void Win32::Service::Handler::stop(bool wait) {

		SERVICE_STATUS status;

		Application::Name appname;

		cout << appname << "\tStopping service" << endl;
		if(!ControlService(handle,SERVICE_CONTROL_STOP,&status)) {

			DWORD error = GetLastError();

			if(error == ERROR_SERVICE_NOT_ACTIVE) {
				clog << appname << "\tService was not active" << endl;
				return;
			}

			throw Win32::Exception("Can't stop service",error);

		}

		if(!(status.dwControlsAccepted & SERVICE_ACCEPT_STOP)) {
			throw runtime_error("The service cant be stopped");
		}

		if(status.dwCurrentState == SERVICE_STOPPED) {
			cout << appname << "\tService is stopped" << endl;
		} else {
			cout << appname << "\tService still running" << endl;
		}

		if(!wait || status.dwCurrentState == SERVICE_STOPPED) {
			return;
		}

		cout << appname << "\tWaiting for service stop"  << endl;

		// Wait for service stop
		for(size_t ix = 0; ix < 100; ix++) {

			Sleep(1000);

#ifdef DEBUG
			cout << "Getting service status"  << endl;
#endif // DEBUG

			if(!QueryServiceStatus(handle,&status)) {
				throw Win32::Exception("Can't get service status");
			}

			if(status.dwCurrentState == SERVICE_STOPPED) {
				cout << appname << "\tService stopped"  << endl;
				return;
			}

		}

		cout << appname << "\tService still running"  << endl;

	}


 }
