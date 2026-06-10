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
 #include <udjat/tools/logger.h>
 #include <private/logger.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/commandlineparser.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/intl.h>
 #include <cstring>
 #include <list>
 #include <ostream>
 #include <iostream>
 #include <vector>
 #include <fstream>      // std::filebuf

 #ifdef _WIN32
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
 #else
	#include <unistd.h>
	#include <syslog.h>
	#include <dlfcn.h>
	#include <sys/resource.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {


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

	void Logger::setup(int &argc, char **argv, bool extract, bool dbg) {

		/*
		String optarg;

		if(dbg) {
			verbosity(9);
			console(true);
		}

		if(CommandLineParser::get_argument(argc,argv,'q',"quiet",optarg,extract)) {
			Logger::console(false);
			Logger::verbosity(optarg.c_str());
		} else if(CommandLineParser::has_argument(argc,argv,'q',"quiet",extract)) {
			Logger::console(false);
		} 
		
		if(CommandLineParser::get_argument(argc,argv,'v',"verbose",optarg,extract)) {
			Logger::console(true);
			Logger::verbosity(optarg.c_str());
		} else if(CommandLineParser::has_argument(argc,argv,'v',"verbose",extract)) {
#ifdef DEBUG
			printf("----> Enabling verbose log\n");
#endif // DEBUG
			Logger::console(true);
		}

		if(CommandLineParser::get_argument(argc,argv,'l',"logfile",optarg,extract)) {
			Logger::file(optarg.c_str());
		} else if(CommandLineParser::has_argument(argc,argv,'l',"logfile",extract)) {
			Logger::file(true);
		}

		if(CommandLineParser::get_argument(argc,argv,'L',"loglevel",optarg,extract)) {
			Logger::verbosity(optarg.c_str());
		} else if(CommandLineParser::has_argument(argc,argv,'L',"loglevel",extract)) {
			Logger::verbosity(Logger::Debug);
		}

#ifndef _WIN32		
		if(CommandLineParser::has_argument(argc,argv,'C',"coredump",extract)) {
			setup_coredump();			
			Logger::String{"Coredump enabled using default pattern"}.info();
		} else if(CommandLineParser::get_argument(argc,argv,'C',"coredump",optarg,extract)) {
			setup_coredump(optarg.c_str());			
			Logger::String{"Coredump enabled using pattern '",optarg.c_str(),"'"}.info();
		}
#endif // !_WIN32	
		*/

	}

	void Logger::help(size_t width) noexcept {

		static const CommandLineParser::Argument values[] = {
			{ 'l', "logfile[=file]", _("Save log to file") },
			{ 'v', "verbose[=verbosity]", _("Send log to console") },
			{ 'L', "loglevel[=verbosity]", _("Set log level to 'verbosity'") },
			{ 'q', "quiet", _("Quiet output") },
#ifndef _WIN32
			{ 'C', "coredump[=pattern]", _("Enable coredump") },
#endif // _WIN32
		};
	
		cout << _("Log/Debug options:\n");
		for(const auto &value : values) {
			value.print(cout,width);
			cout << "\n";
		};

		cout << "\n";

	}


 }

