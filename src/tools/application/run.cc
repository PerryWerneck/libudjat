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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/agent/abstract.h>
 #include <udjat/tools/intl.h>
 #include <udjat/module.h>
 #include <private/updater.h>
 #include <string>
 #include <getopt.h>

 #ifdef _WIN32
	#include <private/win32/mainloop.h>
	#include <udjat/win32/exception.h>
	#include <private/event.h>
 #endif // _WIN32

 using namespace std;

 static const struct {
	char to;
	const char *from;
	const char *help;
 } options[] = {
	{ 'f',	"foreground",	"\t\tRun in foreground with console output" },
	{ 'q',	"quiet",		"\t\tDisable console output" },
	{ 'v',	"verbose",		"=level\tSet loglevel, enable console output" },
	{ 'T',	"timer",		"=time\t\tExit application after \"time\"" },
 };

 namespace Udjat {

	void Application::root(std::shared_ptr<Abstract::Agent>) {

	}

	bool Application::argument(const char *opt, const char *optarg) {

		for(auto &option : options) {
			if(!strcasecmp(opt,option.from)) {
				return argument(option.to,optarg);
			}
		}

		return false;
	}

	bool Application::argument(const char opt, const char *optarg) {

		switch(opt) {
		case 'f':	// Legacy.
			Logger::console(true);
			return true;

		case 'T':
			MainLoop::getInstance().TimerFactory(((time_t) TimeStamp{optarg}) * 1000,[](){
				MainLoop::getInstance().quit("Timer expired, exiting");
				return false;
			});
			return true;

		case 'q':
			Logger::console(false);
			return true;

		case 'v':
		case 'V':
			Logger::console(true);
			if(optarg) {
				if(toupper(*optarg) == 'V') {
					while(toupper(*optarg) == 'V') {
						Logger::verbosity(Logger::verbosity()+1);
						optarg++;
					}
				} else if(optarg[0] >= '0' && optarg[0] <= '9') {
					Logger::verbosity(std::stoi(optarg));
				} else {
					throw runtime_error("Invalid argument value");
				}
			} else {
				Logger::verbosity(Logger::verbosity()+1);
			}
			debug("Verbosity is now '",Logger::verbosity(),"'");
			return true;
		}

		return false;
	}

	bool Application::setProperty(const char *name, const char *value) {

		debug("Property: '",name,"'('",(value ? value : "NULL"),"')");

#ifdef _WIN32
		if(!SetEnvironmentVariable(name,value)) {
			throw Win32::Exception(_("Unable to set environment variable"));
		}
#else
		if(setenv(name, value, 1)) {
			throw std::system_error(errno,std::system_category(),_("Invalid property"));
		}
#endif // _WIN32

		return true;
	}

	void Application::help(std::ostream &out) const noexcept {

		for(auto &option : options) {
			out << "  --" << option.from << option.help << endl;
		}

	}

	int Application::setup(int argc, char **argv, const char *) {

		int ix = 1;
		while(ix < argc) {

			if(String{argv[ix]}.select("-h","--help","/?","-?","help","?",NULL) != -1) {

				Logger::console(false);
				cout << Logger::Message{ _("Usage:\t{} [options]"), argv[0]} << endl << endl;
				help(cout);
				cout << endl << endl;
				return ECANCELED;

			} else if(argv[ix][0] == '-' && argv[ix][1] == '-') {

				// It's a '--name=' argument.
				const char *name = argv[ix]+2;
				const char *value = strchr(name,'=');

				if(value) {
					if(!argument(string{name,(size_t) (value-name)}.c_str(),value+1)) {
						throw std::system_error(EINVAL,std::system_category(),string{name,(size_t) (value-name)});
					}
				} else {
					if(!argument(name)) {
						throw std::system_error(EINVAL,std::system_category(),name);
					}
				}

				ix++;
			} else if(argv[ix][0] == '-') {

				const char name = argv[ix][1];
				ix++;

				// It's a '-N value' argument
				if(ix < argc && argv[ix][0] != '-') {
					if(!argument(name,argv[ix])) {
						throw std::system_error(EINVAL,std::system_category());
					}
					ix++;
				} else {
					if(!argument(name)) {
						throw std::system_error(EINVAL,std::system_category());
					}
				}

			} else {

				const char * name = argv[ix];
				const char * value = strchr(argv[ix],'=');

				if(!value) {
					throw std::system_error(EINVAL,std::system_category());
				}

				if(!setProperty(string{name,(size_t) (value-name)}.c_str(),value+1)) {
					throw runtime_error(_("Invalid property"));
				}

				ix++;

			}

			if(ix >= argc) {
				break;
			}

		}

		return 0;

	}

	int Application::run(int argc, char **argv) {
		return run(argc,argv,nullptr);
	}

	int Application::run(int argc, char **argv, const char *definitions) {

		// Parse command line arguments.
		if(setup(argc,argv,definitions)) {
			return 0;
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


