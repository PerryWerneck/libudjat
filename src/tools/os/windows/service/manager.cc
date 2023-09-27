/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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

 /**
  * @brief Implements Windows Service Manager.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/win32/service.h>
 #include <udjat/win32/exception.h>
 #include <udjat/tools/string.h>

 using namespace std;

 namespace Udjat {

	Win32::Service::Manager::Manager(DWORD dwDesiredAccess) : handle{OpenSCManager(NULL, NULL, dwDesiredAccess)} {
		if(!handle) {
			throw Win32::Exception("Can't open windows service manager");
		}
	}

	Win32::Service::Manager::~Manager() {
		CloseServiceHandle(handle);
	}

	/// @brief Open Service.
	SC_HANDLE Win32::Service::Manager::open(const char *name, DWORD dwDesiredAccess) {
		return OpenService(handle, name, dwDesiredAccess);
	}

	/// @brief Create windows service.
	void Win32::Service::Manager::insert(const char *name, const char *display_name, const char *service_binary) {

		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682450(v=vs.85).aspx
		SC_HANDLE hService = CreateService(	handle,
											TEXT(name),	 							// Service name
											TEXT(display_name),						// Service display name
											SERVICE_ALL_ACCESS,						// Includes STANDARD_RIGHTS_REQUIRED in addition to all access rights in this table.
											SERVICE_WIN32_OWN_PROCESS,				// Service that runs in its own process..
											SERVICE_AUTO_START,						// A service started automatically by the service control manager during system startup.
											SERVICE_ERROR_NORMAL,					// The severity of the error, and action taken, if this service fails to start.
											service_binary,							// The fully-qualified path to the service binary file.
											NULL,									// Load order group
											NULL,									// Group member tab
											NULL,									// Dependencies
											NULL,									// Account
											NULL									// Password
										);

		if(!hService) {
			throw Win32::Exception("Can't create service");
		}

		CloseServiceHandle(hService);

	}

	/// @brief Remove service
	bool Win32::Service::Manager::remove(const char *name) {

		SC_HANDLE hService = OpenService(handle, TEXT(name), SERVICE_ALL_ACCESS);
		if(!hService) {
			if(GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST) {
				return false;
			}
			throw Win32::Exception("Can't open service");
		}

		if(!DeleteService(hService)) {
			// Não consegui remover o serviço
			CloseServiceHandle(hService);
			throw Win32::Exception("Can't delete service");
		}

		CloseServiceHandle(hService);

		return true;

	}

	bool Win32::Service::Manager::start(const char *name) {

		SC_HANDLE hService = OpenService(handle, TEXT(name), SERVICE_ALL_ACCESS);
		if(!hService) {
			auto lasterror = GetLastError();
			if(lasterror == ERROR_SERVICE_DOES_NOT_EXIST) {
				clog << "Service '" << name << "' does not exist" << endl;
				return false;
			}
			throw Win32::Exception("Cant start service",lasterror);
		}

		Win32::Service::Handler{hService}.start();

		return true;

	}

	bool Win32::Service::Manager::stop(const char *name) {

		SC_HANDLE hService = OpenService(handle, TEXT(name), SERVICE_ALL_ACCESS);
		if(!hService) {
			auto lasterror = GetLastError();
			if(lasterror == ERROR_SERVICE_DOES_NOT_EXIST) {
				clog << "Service '" << name << "' does not exist" << endl;
				return false;
			}
			throw Win32::Exception("Cant stop service",lasterror);
		}

		Win32::Service::Handler{hService}.stop();

		return true;

	}

 }

