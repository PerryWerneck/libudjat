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
 #include <udjat/tools/application.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/agent/abstract.h>
 #include <stdexcept>
 #include <udjat/tools/event.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/intl.h>
 #include <udjat/win32/service.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/activatable.h>
 #include <udjat/win32/registry.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	int SystemService::argument(char opt, const char *optstring) {

		switch(opt) {
		case 'h':
			cout	<< "  --start\t\tStart service" << endl
					<< "  --uninstall\t\tStop service" << endl;
			break;

		case 'f':
			mode = Foreground;
			return Application::argument(opt,optstring);

		case 'S':
			return start();

		case 'Q':
			return stop();

		default:
			return Application::argument(opt,optstring);

		}

		return 1;
	}

	void SystemService::status(const char *status) noexcept {

		Logger::write((Logger::Level) (Logger::Trace+1),name().c_str(),status);

		try {

			Win32::Registry registry("service",true);

			registry.set("status",status);
			registry.set("status_time",TimeStamp().to_string().c_str());


		} catch(const std::exception &e) {

			error() << "Cant set service state: " << e.what() << endl;

		}

	}


	/// @brief Initialize service.
	int SystemService::init(const char *definitions) {

		int rc = Application::init(definitions);
		if(rc) {
			debug("Application init rc was ",rc);
			return rc;
		}

		try {

			set(Abstract::Agent::root());

		} catch(const std::exception &e) {

			status(e.what());

		}

		return rc;
	}

	int SystemService::run(const char *definitions) {

		int rc = 0;
		this->definitions = definitions;

		switch(mode) {
		case Foreground:
			rc = Application::run(definitions);
			break;

		case None:
			return 0;

		case Default:
		case Daemon:
			{

				static SERVICE_TABLE_ENTRY DispatchTable[] = {
					{ TEXT(((char *) PACKAGE_NAME)), (LPSERVICE_MAIN_FUNCTION) dispatcher },
					{ NULL, NULL }
				};

				DispatchTable[0].lpServiceName = TEXT((char *) Application::Name::getInstance().c_str());

				if(!StartServiceCtrlDispatcher( DispatchTable )) {
					Logger::String{
						"Failed to start service dispatcher: ",
						Win32::Exception::format(GetLastError())
					}.error("win32");
					return -1;
				}

			}
			break;

		}

		return rc;

	}

	int SystemService::install(const char *description) {

		mode = None;

		Application::Name appname;

		string display_name;
		if(description) {
			display_name = description;
		} else {
			display_name = Application::Description();
		}

		// Get my path
		TCHAR service_binary[MAX_PATH];
		if(!GetModuleFileName(NULL, service_binary, MAX_PATH ) ) {
			throw Win32::Exception("Can't get service filename");
		}

		cout << "Installing service " << appname << " - " << display_name << endl;

		Win32::Service::Manager{}.insert(
			appname.c_str(),
			display_name.c_str(),
			service_binary
		);

		return 1;

	}

	/// @brief Uninstall service.
	/// @return 0 when success, errno if failed.
	/// @retval ENOTSUP No support for this method.
	int SystemService::uninstall() {

		mode = None;

		Application::Name appname;
		Win32::Service::Manager manager;

		if(!Win32::Service::Handler(manager.open(appname.c_str()))) {
			auto lasterror = GetLastError();
			if(lasterror == ERROR_SERVICE_DOES_NOT_EXIST) {
				cout << "Service '" << appname << "' does not exist" << endl;
				return 0;
			}
			throw Win32::Exception("Cant open service",lasterror);
		}

		cout << "Removing service '" << appname << "'" << endl;
		manager.remove(appname.c_str());
		cout << "Service '" << appname << "' removed" << endl;

		return 1;

	}

	/// @brief Start service.
	int SystemService::start() {

		mode = None;

		Application::Name appname;
		cout << "Starting service '" << appname << "'" << endl;

		Win32::Service::Manager manager;
		Win32::Service::Handler service{manager.open(appname.c_str())};

		if(!service) {
			throw Win32::Exception("Cant start service",GetLastError());
		}

		cout << "Starting service '" << appname << "'" << endl;
		service.start();
		cout << "Service '" << appname << "' was started" << endl;

		return 1;

	}

	int SystemService::stop() {

		mode = None;

		Application::Name appname;
		cout << "Stopping service '" << appname << "'" << endl;

		Win32::Service::Manager manager;
		Win32::Service::Handler service{manager.open(appname.c_str())};

		if(!service) {
			auto lasterror = GetLastError();
			if(lasterror == ERROR_SERVICE_DOES_NOT_EXIST) {
				clog << "Service '" << appname << "' does not exist" << endl;
				return 0;
			}
			throw Win32::Exception("Cant stop service",lasterror);
		}

		service.stop();

		return 1;

	}

 }

