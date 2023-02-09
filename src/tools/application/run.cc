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
 #include <udjat/module.h>
 #include <private/updater.h>
 #include <string>
 #include <getopt.h>

 using namespace std;

 namespace Udjat {

	int Application::run(int argc, char **argv, const char *definitions) {

		int rc = 0;

		// Check for command line arguments.
		{
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
			static struct option options[] = {
				{ "verbose",	optional_argument,	0,	'v'	},
				{ "quiet",		no_argument,		0,	'q'	},
			};
			#pragma GCC diagnostic pop

			int long_index =0;
			int opt;
			while((opt = getopt_long(argc, argv, "vq", options, &long_index )) != -1) {

				switch(opt) {
				case 'q':
					Logger::console(false);
					break;

				case 'v':
					Logger::console(true);
					if(optarg) {
						if(*optarg == 'v') {
							while(*optarg == 'v') {
								Logger::verbosity(Logger::verbosity()+1);
								debug("--->",Logger::verbosity());
								optarg++;
							}
						} else {
							Logger::verbosity(std::stoi(optarg));
						}
					} else {
						Logger::verbosity(Logger::verbosity()+1);
					}
					debug("Verbosity is now '",Logger::verbosity(),"'");
					break;

				}

			}

		}

		// Initialize
		try {
			time_t timer = setup(definitions,true);
			if(timer) {
				info() << "Auto-refresh set to " << TimeStamp{time(0)+timer} << endl;

				// TODO: Implement auto-refresh timer.
				error() << "Auto refresh is not implemented, aborting" << endl;
				rc = -1;
 			}
		} catch(const std::exception &e) {
			cerr << e.what() << endl;
			return -1;
		}

		// Run
		if(!rc) {
			rc = MainLoop::getInstance().run();
		}

		// Deinitialize
		try {
			ThreadPool::getInstance().wait();
			Module::unload();
		} catch(const std::exception &e) {
			cerr << e.what() << endl;
			return -1;
		}

		return rc;

	}

 }

