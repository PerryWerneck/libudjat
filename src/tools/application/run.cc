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

	int Application::run(int argc, char **argv, const char *definitions) {

		// Check for command line arguments.
		{
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
			static struct option options[] = {
				{ "verbose",	optional_argument,	0,	'v'	},
				{ "verbosity",	optional_argument,	0,	'V'	},
				{ "daemon",		no_argument,		0,	'D'	},
				{ "quiet",		no_argument,		0,	'q'	},
				{ "help",		no_argument,		0,	'h'	},
				{ "foreground",	no_argument,		0,	'f'	},
				{ "timer",		required_argument,	0,	'T'	},
				{ "install",	no_argument,		0,	'I'	},
				{ "uninstall",	no_argument,		0,	'U'	},
				{ "start",		no_argument,		0,	'S'	},
				{ "stop",		no_argument,		0,	'Q'	},
				{ "reinstall",	no_argument,		0,	'R'	},
			};
			#pragma GCC diagnostic pop

			try {

				int long_index =0;
				int opt;
				while((opt = getopt_long(argc, argv, "vVDqIUhfT:SQR", options, &long_index )) != -1) {

					switch(opt) {
					case 'h':
						cout 	<< "Usage:\t" << argv[0] << " [options]" << endl << endl
								<< "  --help\t\tShow this message" << endl
								<< "  --verbose\t\tSet loglevel, enable console output" << endl
								<< "  --timer\t\tExit after the informed time" << endl
								<< "  --quiet\t\tDisable console output" << endl;
						argument(opt,optarg);
						return 0;

					case 'I':	// Install
						install();
						break;

					case 'U':	// Uninstall
						uninstall();
						break;

					default:
						switch(argument(opt,optarg)) {
						case 0:
							debug("Argument '",opt,"' returned 0, stopping");
							return 0;

						case -1:
							debug("Argument '",opt,"' returned -1, aborting");
							return -1;
						}
					}

				}

			} catch(const std::exception &e) {

				cerr << endl << e.what() << endl << endl;
				return -1;

			}

		}

		Logger::redirect();
		return run(definitions);

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

		} catch(const std::exception &e) {

			error() << e.what() << endl;
			rc = -1;

		}

		return rc;

	}

 }


