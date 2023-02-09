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
 #include <udjat/agent/abstract.h>
 #include <udjat/module.h>
 #include <private/updater.h>
 #include <string>
 #include <getopt.h>

 using namespace std;

 namespace Udjat {

	void Application::root(std::shared_ptr<Abstract::Agent>) {
	}

	int Application::run(int argc, char **argv, const char *definitions) {

		// Check for command line arguments.
		{
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
			static struct option options[] = {
				{ "verbose",	optional_argument,	0,	'v'	},
				{ "quiet",		no_argument,		0,	'q'	},
				{ "install",	no_argument,		0,	'I'	},
				{ "help",		no_argument,		0,	'h'	},
				{ "foreground",	no_argument,		0,	'f'	},
			};
			#pragma GCC diagnostic pop

			int long_index =0;
			int opt;
			while((opt = getopt_long(argc, argv, "vqIhf", options, &long_index )) != -1) {

				switch(opt) {
				case 'h':
					cout 	<< "Usage: " << endl << "  " << argv[0] << " [options]" << endl << endl
							<< "  --help\t\tShow this message" << endl
							<< "  --verbose\t\tSet loglevel, enable console output" << endl
							<< "  --quiet\t\tDisable console output" << endl
							<< "  --install\t\tInstall application" << endl;
					return 0;

				case 'f': // For compatibility with SystemService
					Logger::console(true);
					Logger::verbosity(9);
					break;

				case 'I':
					return install();

				case 'q':
					Logger::console(false);
					break;

				case 'v':
					Logger::console(true);
					if(optarg) {
						if(*optarg == 'v') {
							while(*optarg == 'v') {
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

			}

		}

		return run(definitions);
	}

	int Application::run(const char *definitions) {

			// Initialize
			{
				time_t timer = setup(definitions,true);
				if(timer) {
					info() << "Auto-refresh set to " << TimeStamp{time(0)+timer} << endl;

					// TODO: Implement auto-refresh timer.
					error() << "Auto update is not implemented, aborting" << endl;
					return ENOTSUP;
				}
			}

			int rc = -1;

			try {

				root(Abstract::Agent::root());	// throw if the agent subsystem is inactive.
				MainLoop::getInstance().run();
				ThreadPool::getInstance().wait();
				Module::unload();

			} catch(const std::exception &e) {

				error() << e.what() << endl;
				rc = -1;

			}

			return rc;

	}

 }


