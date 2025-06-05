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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/commandlineparser.h>
 #include <udjat/agent/abstract.h>
 #include <udjat/tools/intl.h>
 #include <udjat/module/abstract.h>
 #include <udjat/ui/console.h>
 #include <private/updater.h>
 #include <string>
 #include <getopt.h>

 #undef LOG_DOMAIN
 #define LOG_DOMAIN Application::Name();
 #include <udjat/tools/logger.h>

 #include <iostream>     // std::cout, std::ostream, std::ios
 #include <fstream>      // std::filebuf

 #ifdef _WIN32
	#include <private/win32/mainloop.h>
	#include <udjat/win32/exception.h>
	#include <private/event.h>
 #else
	#include <sys/resource.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	void Application::root(std::shared_ptr<Abstract::Agent>) {
	}

	bool Application::setProperty(const char *name, const char *value) {

		debug("Property: '",name,"'('",(value ? value : "NULL"),"')");

#ifdef _WIN32
		if(!SetEnvironmentVariable(name,value)) {
			throw Win32::Exception(_("Unable to set environment variable"));
		}
#else
		if(setenv(name, value, 1)) {
			throw std::system_error(errno,std::system_category(),_("Unable to set environment variable"));
		}
#endif // _WIN32

		return true;
	}

	int Application::setup(const char *) {

		debug("---> Running ",__FUNCTION__);

		string argvalue;

		if(CommandLineParser::get_argument(argc,argv,'T',"timer",argvalue)) {
			MainLoop::getInstance().TimerFactory(((time_t) TimeStamp{argvalue.c_str()}) * 1000,[](){
				MainLoop::getInstance().quit("Timer expired, exiting");
				return false;
			});
		}
				
		return 0;

	}

	int Application::run(const char *definitions) {

		if(has_argument('h',"help",true)) {
			help();
			cout << "\n";
			Logger::help();
			return 0;
		}

		CommandLineParser::setup(argc,argv);

		// Parse command line arguments.
		if(setup(definitions)) {
			return -1;
		}

		if(!MainLoop::getInstance()) {
			return 0;
		}

		int rc = -1;

		try {

			rc = init(definitions);
			if(rc) {
				return rc;
			}

#ifdef _WIN32
			Udjat::Event::ConsoleHandler(this,CTRL_C_EVENT,[](){
				MainLoop::getInstance().quit("Terminating by ctrl-c event");
				return false;
			});

			Udjat::Event::ConsoleHandler(this,CTRL_CLOSE_EVENT,[](){
				MainLoop::getInstance().quit("Terminating by close event");
				return false;
			});

			Udjat::Event::ConsoleHandler(this,CTRL_SHUTDOWN_EVENT,[](){
				MainLoop::getInstance().quit("Terminating by shutdown event");
				return false;
			});
#endif // _WIN32

			rc = MainLoop::getInstance().run();

			if(deinit(definitions)) {
				rc = -1;
			}

			finalize();

		} catch(const std::exception &e) {

			error() << e.what() << endl;
			rc = -1;

		}

		return rc;

	}

 }


