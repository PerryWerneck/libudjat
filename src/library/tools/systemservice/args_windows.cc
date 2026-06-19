/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/argumentparser.h>
 #include <udjat/tools/intl.h>
 #include <udjat/win32/registry.h>

 using namespace std;

 namespace Udjat {

	ArgumentParser & SystemService::load(ArgumentParser &parser) noexcept {
		parser.append(
			_("Service options"),
				ArgumentParser::Argument{
					'F', "foreground", _("Run in foreground, as application"),
					[this](const char *, char) {
						Application::run();
						return true;
					}
				},
				ArgumentParser::Argument{
					'S', "start", _("Start service"),
					[this](const char *, char) {
						start();
						return true;
					}
				},
				ArgumentParser::Argument{
					'Q', "stop", _("Stop service"),
					[this](const char *, char) {
						stop();
						return true;
					}
				},
				ArgumentParser::Argument{
					'I', "install", _("Install service"), _("description"),
					[this](const char *description, char) {
						install(description);
						return true;
					}
				},
				ArgumentParser::Argument{
					'U', "uninstall", _("Uninstall service"),
					[this](const char *, char) {
						uninstall();
						return true;
					}
				},
				ArgumentParser::Argument{
					'R', "reinstall", _("Reinstall service"),
					[this](const char *, char) {
						stop();
						uninstall();
						install();
						start();
						return true;
					}
				},
				ArgumentParser::Argument{
					'B', "unstoppable", _("Block access to 'net stop' on this service"),
					[this](const char *, char) {
						Application::Name appname;
						Win32::Service::Manager{}.setUnStoppable(appname.c_str());
						return true;
					}
				}
		);
		return parser;
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


 }

