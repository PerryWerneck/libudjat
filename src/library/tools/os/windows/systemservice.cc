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

	/// @brief Show help text to stream.
	/// @param out The stream for help.
	void SystemService::help(size_t width) const noexcept {

		Application::help(width);

		static const CommandLineParser::Argument values[] = {
			{ 'f',	"foreground",	_("Run in foreground (as application)")				},
			{ 'S',	"start",		_("Start service")									},
			{ 'Q',	"stop",			_("Stop service")									},
			{ 'I',	"install",		_("Install service")								},
			{ 'U',	"uninstall",	_("Uninstall service")								},
			{ 'R',	"reinstall",	_("Reinstall service")								},
			{ 'B',	"unstoppable",	_("Block access to 'net stop' on this service")		},
		};
		
		for(const auto &value : values) {
			value.print(cout,width);
			cout << "\n";
		};

	}

	/// @brief Initialize service.
	int SystemService::init(const char *definitions) {
		return Application::init(definitions);
	}

	void SystemService::on_timer() {
	}

	Dialog::Status & SystemService::state(const Level level, const char *message) noexcept {

		Logger::write((Logger::Level) (Logger::Trace+1),name().c_str(),message);

		try {

			Win32::Registry registry("service",true);

			registry.set("status",message);
			registry.set("status_time",TimeStamp().to_string().c_str());

		} catch(const std::exception &e) {

			error() << "Cant set service state: " << e.what() << endl;

		}

		return Application::state(level,message);

	}

	int SystemService::install(const char *description) {

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

	int SystemService::uninstall() {

		Application::Name appname;

		if(Win32::Service::Manager{}.remove(appname.c_str())) {
			cout << "Service '" << appname << "' removed" << endl;
		} else {
			cout << "Service '" << appname << "' does not exist" << endl;
		}

		return 1;

	}

	int SystemService::start() {

		Application::Name appname;

		if(Win32::Service::Manager{}.start(appname.c_str())) {
			cout << "Service '" << appname << "' was started" << endl;
		} else {
			cerr << "Service '" << appname << "' was NOT started" << endl;
		}

		return 1;

	}

	int SystemService::stop() {

		Application::Name appname;

		if(Win32::Service::Manager{}.stop(appname.c_str())) {
			cout << "Service '" << appname << "' was stopped" << endl;
		} else {
			cerr << "Service '" << appname << "' was NOT stopped" << endl;
		}

		return 1;

	}

	int SystemService::run(const char *definitions) {

		bool exit = false;

		if(has_argument('h',"help",true)) {
			help();
			cout << "\n";
			Logger::help();
			return 0;
		}

		if(has_argument('I',"install",true)) {
			exit = true;
			int rc = install();
			if(rc) {
				return rc;
			}
		}
		
		if(has_argument('U',"uninstall",true)) {
			exit = true;
			int rc = uninstall();
			if(rc) {
				return rc;
			}
		}

		if(has_argument('R',"reinstall",true)) {
			exit = true;
			stop();
			uninstall();
			install();
			start();
		}

		if(has_argument('B',"unstoppable",true)) {
			exit = true;
			Application::Name appname;
			Win32::Service::Manager{}.setUnStoppable(appname.c_str());
		} 
		
		if(has_argument('f',"foreground",true)) {
			return Application::run();
		}

		if(has_argument('S',"start",true)) {
			exit = true;
			int rc = start();
			if(rc) {
				return rc;
			}
		} 
		
		if(has_argument('Q',"stop",true)) {
			exit = true;
			int rc = stop();
			if(rc) {
				return rc;
			}
		}

		if(!exit) {

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

		}

		return 0;

	}

 }

