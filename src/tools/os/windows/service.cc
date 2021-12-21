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

 using namespace std;

 namespace Udjat {

	void Win32::Service::stop(bool wait) {

		SERVICE_STATUS status;

		cout << "win32\tStopping service" << endl;
		if(!ControlService(handle,SERVICE_CONTROL_STOP,&status)) {

			DWORD error = GetLastError();

			if(error == ERROR_SERVICE_NOT_ACTIVE) {
				clog << "win32\tService was not active" << endl;
				return;
			}

			throw Win32::Exception("Can't stop service",error);

		}

		if(!(status.dwControlsAccepted & SERVICE_ACCEPT_STOP)) {
			throw runtime_error("The service cant be stopped");
		}

		if(status.dwCurrentState == SERVICE_STOPPED) {
			cout << "win32\tService is stopped" << endl;
		} else {
			cout << "win32\tService still running" << endl;
		}

		if(!wait || status.dwCurrentState == SERVICE_STOPPED) {
			return;
		}

		cout << "win32\tWaiting for service stop"  << endl;

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
				cout << "win32\tService stopped"  << endl;
				return;
			}

		}

		cout << "win32\tService still running"  << endl;

	}

	int Win32::Service::install(const char *name, const char *display_name, const char *service_binary, bool start) {

		try {

			ServiceManager manager;

			try {
				Service(manager.open(name)).stop();
			} catch(const exception &e) {
				cerr << "win32\tCant stop service: " << e.what() << endl;
			}

			try {
				manager.remove(name);
			} catch(const exception &e) {
				cerr << "win32\tCant remove service: " << e.what() << endl;
			}

			// Add service
			Win32::Service service(manager.insert(name,display_name,service_binary));

			if(start) {
				service.start();
			}

		} catch(const exception &e) {

			cerr << "win32\tError '" << e.what() << "' installing service" << endl;
			return -1;
		}

		return 0;
	}


 }
