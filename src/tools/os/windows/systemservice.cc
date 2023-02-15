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

		default:
			return Application::argument(opt,optstring);

		}

		return 1;
	}

	void SystemService::status(const char *status) noexcept {

		try {

			Win32::Registry registry("service",true);

			registry.set("status",message);
			registry.set("status_time",TimeStamp().to_string().c_str());

			Logger::write((Logger::Level) (Logger::Trace+1),name().c_str(),message);

		} catch(const std::exception &e) {

			error() << "Cant set service state: " << e.what() << endl;

		}

	}


	/// @brief Initialize service.
	int SystemService::init(const char *definitions) {

		int rc = Application::init(definitions);

		try {

			set(Abstract::Agent::root());

		} catch(const std::exception &e) {

			status(e.what());

		}

		return rc;
	}

	int SystemService::run(const char *definitions) {

		int rc = 0;

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
					{ TEXT(((char *) PACKAGE_NAME)), (LPSERVICE_MAIN_FUNCTION) Win32::Service::Controller::dispatcher },
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


 }

