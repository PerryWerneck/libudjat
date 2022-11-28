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
 #include <private/protocol.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/intl.h>
 #include <udjat/moduleinfo.h>
 #include <cstdio>
 #include <iostream>
 #include <sstream>
 #include <fcntl.h>
 #include <unistd.h>

 using namespace std;

 namespace Udjat {

	static const ModuleInfo moduleinfo { N_("Subprocess/Script protocol" ) };

	Protocol::Controller::Script::Script() : Udjat::Protocol((const char *) "script",moduleinfo) {
	}

	Protocol::Controller::Script::~Script() {
	}

	std::shared_ptr<Protocol::Worker> Protocol::Controller::Script::WorkerFactory() const {

		class Worker : public Protocol::Worker {
		private:
			string path() const {

				if(strncasecmp(url().c_str(),"script://.",10) == 0) {
					return url().c_str()+10;
				}

				return url().ComponentsFactory().path;
			}


		public:
			Worker() = default;

			String get(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) {

				string script = this->path();

				if(access(script.c_str(),R_OK)) {
					throw system_error(ENOENT,system_category(),script);
				}

				class Process : public SubProcess, public stringstream {
				public:
					int rc = 0;
					Process(const char *command) : SubProcess(command) {
					}

				private:
					void onStdOut(const char *line) override {
						*this << line << endl;
					}

					void onStdErr(const char *line) override {
						*this << line << endl;
					}

					void onExit(int rc) override {
						this->rc = rc;
					}

					void onSignal(int UDJAT_UNUSED(sig)) override {
						this->rc = -1;
					}

				};

				Process proc(script.c_str());
				proc.run();

				return String(proc.str());

			}

			int test(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) noexcept override {

				if(method() != HTTP::Head) {
					return EINVAL;
				}

				string script = this->path();

				if(access(script.c_str(),F_OK)) {
					clog << "script\t" << script << " is not available" << endl;
					return 404;
				}

				if(access(script.c_str(),R_OK)) {
					clog << "script\t" << script << " is not acessible" << endl;
					return 401;
				}

				int rc = SubProcess::run(script.c_str());

				if(rc == 0) {
					return 200;
				}

				return 500;
			}

		};

		return make_shared<Worker>();
	}

 }

