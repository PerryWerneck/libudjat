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
 #include <udjat/agent.h>
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

	void SystemService::on_timer() {
	}

	Dialog::Status & SystemService::state(const Level level, const char *message) noexcept {

		Logger::String{message}.write(Logger::Notice,name().c_str());

		try {

			Win32::Registry registry("service",true);

			registry.set("status",message);
			registry.set("status_time",TimeStamp().to_string().c_str());

		} catch(const std::exception &e) {

			error() << "Cant set service state: " << e.what() << endl;

		}

		return Application::state(level,message);

	}

	int SystemService::run(const char *) {
		
		// Run as windows service
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

		return 0;

	}

 }

