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
 #include <udjat/module.h>
 #include <private/updater.h>
 #include <string>
 #include <getopt.h>

 #ifdef _WIN32
	#include <private/win32/mainloop.h>
	#include <private/event.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	void Application::root(std::shared_ptr<Abstract::Agent>) {

	}

	/*
	int Application::argument(char opt, const char *optstring) {

		switch(opt) {
		case 'T':
			MainLoop::getInstance().TimerFactory(((time_t) TimeStamp{optstring}) * 1000,[](){
				MainLoop::getInstance().quit("Timer expired, exiting");
				return false;
			});
			break;

		case 'h':
#ifdef _WIN32
			cout	<< "  --install\t\tInstall" << endl
					<< "  --uninstall\t\tUninstall" << endl;
#endif // _WIN32
			break;

		case 'f':
			Logger::console(true);
			break;

		case 'I':
			return install();

		case 'U':
			return uninstall();

		case 'q':
			Logger::console(false);
			break;

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
					cerr << strerror(EINVAL) << endl;
					return EINVAL;
				}
			} else {
				Logger::verbosity(Logger::verbosity()+1);
			}
			debug("Verbosity is now '",Logger::verbosity(),"'");
			break;

		}

		return 1;

	}
	*/

	bool Application::argument(const char *opt, const char *optarg) {

		static const struct {
			char to;
			const char *from;
		} options[] = {
			{ 'f',	"foreground" },
			{ 'q',	"quiet" },
			{ 'v',	"verbose" },
			{ 'T',	"timer" },
		};

		for(auto &option : options) {
			debug("opt=",opt," from=",option.from," to=",option.to);
			if(!strcasecmp(opt,option.from)) {
				return argument(option.to,optarg);
			}
		}

		return false;
	}

	bool Application::argument(const char opt, const char *optarg) {

		switch(opt) {
		case 'f':
			Logger::console(true);
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

		if(setenv(name, value, 1)) {
			throw std::system_error(errno,std::system_category(),"Invalid property");
		}

		return true;
	}

	int Application::run(int argc, char **argv, const char *definitions) {

		// Parse command line arguments.
		{
			int ix = 1;
			while(ix < argc) {

				if(argv[ix][0] == '-' && argv[ix][1] == '-') {

					// It's a '--name=' argument.
					const char *name = argv[ix]+2;
					const char *value = strchr(name,'=');

					if(value) {
						if(!argument(string{name,(size_t) (value-name)}.c_str(),value+1)) {
							throw runtime_error(string{name,(size_t) (value-name)} + ": Invalid argument");
						}
					} else {
						if(!argument(name)) {
							throw runtime_error(string{name} + ": Invalid argument");
						}
					}

					ix++;
				} else if(argv[ix][0] == '-') {

					const char name = argv[ix][1];
					ix++;

					// It's a '-N value' argument
					if(ix < argc && argv[ix][0] != '-') {
						if(!argument(name,argv[ix])) {
							throw runtime_error("Invalid argument");
						}
						ix++;
					} else {
						if(!argument(name)) {
							throw runtime_error(string{name} + ": Invalid argument");
						}
					}

				} else {

					const char * name = argv[ix];
					const char * value = strchr(argv[ix],'=');

					if(!value) {
						throw runtime_error("Invalid argument");
					}

					if(!setProperty(string{name,(size_t) (value-name)}.c_str(),value+1)) {
						throw runtime_error("Invalid property");
					}

					ix++;

				}

				if(ix >= argc) {
					break;
				}

			}

		}

		Logger::redirect();
		return run(definitions);

		return 0;

	}

	int Application::run(const char *definitions) {

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


