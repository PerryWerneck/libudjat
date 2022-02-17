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
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/application.h>
 #include <iostream>
 #include <udjat-internals.h>

 using namespace std;

 namespace Udjat {

	SystemService * SystemService::instance = nullptr;

	void SystemService::reconfigure(const char *pathname) noexcept {

		try {

			Application::DataFile path(pathname);
			Application::info() << "Loading settings from '" << path << "'" << endl;

			Udjat::load_modules(path.c_str());
			time_t next = Udjat::refresh_definitions(path.c_str());
			Udjat::load_modules(path.c_str());

			auto root = RootFactory();
			load_agent_definitions(root,path.c_str());

			setRootAgent(root);

			if(next) {
				Udjat::MainLoop::getInstance().insert(definitions,next*1000,[]{
					ThreadPool::getInstance().push([]{
						if(instance) {
							instance->reconfigure(instance->definitions);
						}
					});
					return false;
				});

				Application::info() << "Next reconfiguration set to " << TimeStamp(time(0)+next) << endl;
			}

		} catch(const std::exception &e ) {

			Application::error() << "Reconfiguration has failed: " << e.what() << endl;

		} catch(...) {

			Application::error() << "Unexpected error during reconfiguration" << endl;

		}

	}

	std::shared_ptr<Abstract::Agent> SystemService::RootFactory() const {
		return Udjat::RootAgentFactory();
	}

	int SystemService::cmdline(const char *appname, int argc, const char **argv) {

		int exit = 0;
		while(--argc > 0) {

			const char *arg = *(++argv);
			int rc = 0;

			try {

				if(!(strcmp(arg,"-h") && strcmp(arg,"--help") && strcmp(arg,"/h") && strcmp(arg,"/?") && strcmp(arg,"-?"))) {
					usage(appname);
					cout << endl;
					return 0;
				}

				if(arg[0] == '-' && arg[1] == '-') {

					// --param=value

					arg += 2;
					const char *ptr = strchr(arg,'=');
					if(ptr) {
						rc = cmdline(appname,string(arg,ptr-arg).c_str(),ptr+1);
					} else {
						rc = cmdline(appname,arg);
					}
				} else if(arg[0] == '-' && arg[1] && arg[2] == 0) {

					// -P value
					if(argc > 1 && argv[1] && argv[1][0] != '-') {
						rc = cmdline(appname, arg[1], *(++argv));
						argc--;
					} else {
						rc = cmdline(appname,arg[1]);
					}

				} else if(arg[0] == '/' && arg[1] && arg[2] == 0) {

					// /P value
					rc = cmdline(appname,arg[1]);

				}

				if(rc == ENOENT) {
					cerr << "Invalid option: '" << arg << "'" << endl;
					return -1;
				} else if(rc == -2) {
					exit = -2;
				}

			} catch(const std::exception &e) {

				cerr << appname << "\t" << e.what() << endl;
				return -1;

			} catch(...) {

				cerr << appname << "\tUnexpected error" << endl;
				return -1;

			}

		}

		return exit;
	}

 }
