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

 #include "../private.h"

 #ifndef _WIN32

 #include <udjat/tools/url.h>
 #include <udjat/moduleinfo.h>
 #include <cstdio>
 #include <iostream>
 #include <sstream>
 #include <fcntl.h>
 #include <unistd.h>

 using namespace std;

 namespace Udjat {

	static const ModuleInfo moduleinfo { "Script protocol module" };

	Protocol::Controller::Script::Script() : Udjat::Protocol((const char *) "script",moduleinfo) {
	}

	Protocol::Controller::Script::~Script() {
	}

	std::shared_ptr<Protocol::Worker> Protocol::Controller::Script::WorkerFactory() const {

		class Worker : public Protocol::Worker {
		public:
			Worker() = default;

			string path() const {

				if(strncasecmp(url().c_str(),"script://.",10) == 0) {
					return url().c_str()+10;
				}

				return url().ComponentsFactory().path.c_str();
			}

			String get(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) {

				const char *script = url().ComponentsFactory().path.c_str();
				FILE *in = popen(script, "r");
				if(!in) {
					throw system_error(errno,system_category(),script);
				}

				stringstream rsp;

				try {

					int ch;
					while( (ch=fgetc(in)) != EOF) {
						rsp << ((char) ch);
					}

				} catch(...) {
					pclose(in);
					throw;
				}

				int rc = pclose(in);
				clog << "script\t" << script << " rc=" << rc << endl;

				return String(rsp.str());
			}

			unsigned short test() override {

				const char *script = url().ComponentsFactory().path.c_str();

				if(access(script,R_OK)) {
					clog << "script\t" << script << " is not available" << endl;
					return 404;
				}

				int rc = system(script);

				if(rc == 0)
					return 200;

				clog << "script\t" << script << " rc=" << rc << endl;

				return 500;
			}

		};

		return make_shared<Worker>();
	}

 }
 #endif // !_WIN32

