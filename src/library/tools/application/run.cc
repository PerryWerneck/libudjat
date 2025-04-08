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

 /*
 static const struct {
	char to;
	const char *from;
	const char *help;
 } options[] = {
	{ 'f',	"foreground",	"\t\tRun in foreground with console output" },
	{ 'q',	"quiet",		"\t\tDisable console output" },
	{ 'v',	"verbose",		"=level\tSet loglevel, enable console output" },
#ifndef _WIN32
	{ 'C',	"coredump",		"=pattern\tEnable coredump" },
#endif // _WIN32
	{ 'T',	"timer",		"=time\t\tExit application after \"time\"" },
 };
 */

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

	void Application::help(std::ostream &out) const noexcept {

		out << "  -h --help                Show this help" << endl;
		out << "  -f --foreground          Run in foreground with console output" << endl;
		out << "  -q --quiet               Disable console output" << endl;
		out << "  -v --verbose[=level]     Set loglevel, enable console output" << endl;
		out << "  -C --coredump[=pattern]  Enable coredump" << endl;
		out << "  -T --time=time           Exit application after \"time\"" << endl;

	}

#ifndef _WIN32
	static void setup_coredump(const char *pattern = nullptr) {
		// Reference script:
		//
		// ulimit -c unlimited
		// install -m 1777 -d /var/local/dumps
		// echo "/var/local/dumps/core.%e.%p"> /proc/sys/kernel/core_pattern
		// rcapparmor stop
		// sysctl -w kernel.suid_dumpable=2
		//

		struct rlimit core_limits;
		memset(&core_limits,0,sizeof(core_limits));

		core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
		setrlimit(RLIMIT_CORE, &core_limits);

		if(pattern && *pattern) {
			// Set corepattern
			std::filebuf fb;
			fb.open ("/proc/sys/kernel/core_pattern",std::ios::out);
			std::ostream os(&fb);
			os << pattern << "\n";
			fb.close();
		}
	}

#endif // !_WIN32

	int Application::setup(int argc, char **argv, const char *) {

		debug("---> Running ",__FUNCTION__);

		string argvalue;

		while(popup(argc,argv,'v',"verbose")) {
			Logger::verbosity(Logger::verbosity()+1);
		}

		if(popup(argc,argv,'f',"foreground")) {
			Logger::console(true);
		}

		if(popup(argc,argv,0,"debug")) {
			Logger::enable(Logger::Debug);
		}

		if(popup(argc,argv,0,"trace")) {
			Logger::enable(Logger::Trace);
		}

		if(popup(argc,argv,'q',"quiet")) {
			Logger::console(false);
		}

#ifndef _WIN32
		if(popup(argc,argv,'C',"coredump")) {
			setup_coredump();
		}
		
		if(popup(argc,argv,'C',"coredump",argvalue)) {
			setup_coredump(argvalue.c_str());
		}
#endif // _WIN32

		if(popup(argc,argv,'T',"timer",argvalue)) {
			MainLoop::getInstance().TimerFactory(((time_t) TimeStamp{argvalue.c_str()}) * 1000,[](){
				MainLoop::getInstance().quit("Timer expired, exiting");
				return false;
			});
		}

		return 0;

	}

	int Application::run(int argc, char **argv) {
		return run(argc,argv,nullptr);
	}

	int Application::run(int argc, char **argv, const char *definitions) {

		if(popup(argc,argv,'h',"help")) {

			// Show help text.
			Udjat::UI::Console console;

			console << "Usage: " << argv[0] << " [options]" << endl << endl << "Options:" << endl;			
			help(console);
			console << "     --debug               Enable debug messages" << endl;
			console << "     --trace               Enable trace messages" << endl;
	//		console << "     --version             Show version" << endl;
			return -2;
		}

		Logger::redirect();

		// Parse command line arguments.
		if(setup(argc,argv,definitions)) {
			return -1;
		}

		Logger::redirect();
		return run(definitions);

	}

	int Application::run(const char *definitions) {

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


