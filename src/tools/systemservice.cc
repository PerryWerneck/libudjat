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
 #include <udjat/module.h>
 #include <iostream>
 #include <udjat-internals.h>

 using namespace std;

 namespace Udjat {

	SystemService * SystemService::instance = nullptr;

	SystemService::SystemService(const char *d) : definitions(d) {

		if(instance) {
			throw runtime_error("Can't start more than one system service");
		}

		setlocale( LC_ALL, "" );
		instance = this;

	}

	SystemService::~SystemService() {
		instance = nullptr;
	}

	SystemService * SystemService::getInstance() {
		if(instance) {
			return instance;
		}
		throw runtime_error("There's no active service instance");
	}

	std::shared_ptr<Abstract::Agent> SystemService::RootFactory() const {
		return Udjat::RootAgentFactory();
	}

	int SystemService::cmdline(int argc, const char **argv) {

		int exit = 0;
		while(--argc > 0) {

			const char *arg = *(++argv);
			int rc = 0;

			try {

				if(!(strcmp(arg,"-h") && strcmp(arg,"--help") && strcmp(arg,"/h") && strcmp(arg,"/?") && strcmp(arg,"-?"))) {
					usage();
					cout << endl;
					return 0;
				}

				if(arg[0] == '-' && arg[1] == '-') {

					// --param=value

					arg += 2;
					const char *ptr = strchr(arg,'=');
					if(ptr) {
						rc = cmdline(string(arg,ptr-arg).c_str(),ptr+1);
					} else {
						rc = cmdline(arg);
					}
				} else if(arg[0] == '-' && arg[1] && arg[2] == 0) {

					// -P value
					if(argc > 1 && argv[1] && argv[1][0] != '-') {
						rc = cmdline(arg[1], *(++argv));
						argc--;
					} else {
						rc = cmdline(arg[1]);
					}

				} else if(arg[0] == '/' && arg[1] && arg[2] == 0) {

					// /P value
					rc = cmdline(arg[1]);

				}

				if(rc == ENOENT) {
					cerr << "Invalid option: '" << arg << "'" << endl;
					return -1;
				} else if(rc == -2) {
					exit = -2;
				}

			} catch(const std::exception &e) {

				cerr << name() << "\t" << e.what() << endl;
				return -1;

			} catch(...) {

				cerr << name() << "\tUnexpected error" << endl;
				return -1;

			}

		}

		return exit;
	}

	std::shared_ptr<Abstract::State> SystemService::state() const {

		std::shared_ptr<Abstract::State> selected;

		for(auto state : states) {
			if(!selected || selected->level() < state->level()) {
				selected = state;
			}
		}

		if(!selected) {
			selected = make_shared<Abstract::State>("ready",Level::unimportant,"No messages","Service is running with no messages");
		}

		return selected;

	}

	void SystemService::activate(std::shared_ptr<Abstract::State> state) {
		states.push_back(state);
		notify(*this->state());
	}

	void SystemService::deactivate(std::shared_ptr<Abstract::State> state) {
		states.remove_if([state](std::shared_ptr<Abstract::State> st){
			return (st.get() == state.get());
		});
		notify(*this->state());

	}


 }
