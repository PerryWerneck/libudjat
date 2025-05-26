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

 static const struct {
	char to;
	const char *from;
	const char *help;
 } options[] = {
	{ 'S',	"start",		N_("\t\tStart service")									},
	{ 'Q',	"stop",			N_("\t\tStop service")									},
	{ 'I',	"install",		N_("\t\tInstall service")								},
	{ 'U',	"uninstall",	N_("\t\tUninstall service")								},
	{ 'R',	"reinstall",	N_("\t\tReinstall service")								},
	{ 'L',	"unstoppable",	N_("\t\tBlock access to 'net stop' on this request")	},
 };

 namespace Udjat {

	bool SystemService::argument(const char *opt, const char *optarg) {

		for(auto &option : options) {
			if(!strcasecmp(opt,option.from)) {
				return argument(option.to,optarg);
			}
		}

		return Application::argument(opt,optarg);
	}

	bool SystemService::argument(const char opt, const char *optarg) {

		switch(opt) {
		case 'f':
			mode = Foreground;
			return Application::argument(opt,optarg);

		case 'S':
			start();
			MainLoop::getInstance().quit();
			break;

		case 'Q':
			stop();
			MainLoop::getInstance().quit();
			break;

		case 'I':
			install();
			MainLoop::getInstance().quit();
			break;

		case 'U':
			uninstall();
			MainLoop::getInstance().quit();
			break;

		case 'R':
			stop();
			uninstall();
			install();
			start();
			MainLoop::getInstance().quit();
			break;

		case 'L':	// Unstoppable
			{
				Application::Name appname;
				Win32::Service::Manager{}.setUnStoppable(appname.c_str());
			}
			break;

		default:
			return Application::argument(opt,optarg);

		}

		return true;
	}

	void SystemService::help(std::ostream &out) const noexcept {

		Application::help(out);

		for(auto &option : options) {
#ifdef GETTEXT_PACKAGE
			out << "  --" << option.from << dgettext(GETTEXT_PACKAGE,option.help) << endl;
#else
			out << "  --" << option.from << option.help << endl;
#endif // GETTEXT_PACKAGE

		}

	}

	Dialog::Status & SystemService::state(const Level, const char *status) noexcept {

		Logger::write((Logger::Level) (Logger::Trace+1),name().c_str(),status);

		try {

			Win32::Registry registry("service",true);

			registry.set("status",status);
			registry.set("status_time",TimeStamp().to_string().c_str());


		} catch(const std::exception &e) {

			error() << "Cant set service state: " << e.what() << endl;

		}

		return Application::state(level,message);
	}

	/// @brief Initialize service.
	int SystemService::init(const char *definitions) {

		return Application::init(definitions);

	}

	int SystemService::run(const char *definitions) {

		if(pop('h',"help")) {

			cout << _("Usage:") << "\n  " << argv[0]
				<< " " << _("[OPTION..]") << "\n\n";

			help();
			cout << "\n";
			
			Logger::help();

			return 0;
		}

		if(!MainLoop::getInstance()) {
			return 0;
		}

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
	int SystemService::uninstall() {

		mode = None;

		Application::Name appname;

		if(Win32::Service::Manager{}.remove(appname.c_str())) {
			cout << "Service '" << appname << "' removed" << endl;
		} else {
			cout << "Service '" << appname << "' does not exist" << endl;
		}

		return 1;

	}

	/// @brief Start service.
	int SystemService::start() {

		mode = None;

		Application::Name appname;

		if(Win32::Service::Manager{}.start(appname.c_str())) {
			cout << "Service '" << appname << "' was started" << endl;
		} else {
			cerr << "Service '" << appname << "' was NOT started" << endl;
		}

		return 1;

	}

	int SystemService::stop() {

		mode = None;

		Application::Name appname;

		if(Win32::Service::Manager{}.stop(appname.c_str())) {
			cout << "Service '" << appname << "' was stopped" << endl;
		} else {
			cerr << "Service '" << appname << "' was NOT stopped" << endl;
		}

		return 1;

	}

 }

